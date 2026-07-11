#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace lf // lock free
{
    // Bounded, wait-free-ish, multi-producer / multi-consumer (MPMC) queue.
    //
    // This is the classic Dmitry Vyukov MPMC ring-buffer design: a fixed
    // capacity array of "Cells", each carrying its own atomic sequence
    // number. The sequence number is what lets many producers and many
    // consumers coordinate on which slot they own *without* a global lock
    // and without the ABA problems a naive index-only ring buffer would
    // have.
    //
    // How the sequence numbers work
    // ------------------------------
    // Slot i starts with sequence == i ("ready for the first push at
    // global position i"). Global positions are 64-bit counters that only
    // ever increase (enqueuePos_ / dequeuePos_); the actual array index is
    // always `pos & mask_`.
    //
    //   - A producer may claim global position `pos` only while
    //     cell.sequence == pos. After constructing the element it sets
    //     cell.sequence = pos + 1 ("ready to be popped").
    //   - A consumer may claim global position `pos` only while
    //     cell.sequence == pos + 1. After destroying/moving-out the
    //     element it sets cell.sequence = pos + capacity ("ready for the
    //     push of the *next* lap through this slot", i.e. global position
    //     pos + capacity).
    //
    // Because every slot's "ready" value differs from its "empty" value
    // by exactly the queue capacity, producers and consumers never
    // observe a slot as available in the wrong direction, so lost updates
    // / ABA that a plain index-CAS ring buffer would suffer from are
    // avoided.
    //
    // Requirements this implementation places on T (see static_asserts
    // below for *why*):
    //   - Constructible from whatever argument try_push is called with.
    //   - That construction must be non-throwing.
    //   - Move-assignable, non-throwing.
    //   - Destructible, non-throwing (a standard assumption almost every
    //     type satisfies; std::terminate is the usual outcome otherwise).
    template<typename T>
    class Queue
    {
        //      Cell 0                 Cell 1                 Cell 2                 Cell 3
        //     + ---------- +        +---------- +           +---------- +          +---------- +
        //     | seq = 0  |           | seq = 1  |           | seq = 2  |           | seq = 3  |
        //     | storage  |           | storage  |           | storage  |           | storage |
        //     +---------- +         +---------- +           +---------- +          +---------- +
        // Cell = sequence + raw memory for T
        struct Cell
        {
            // The sequence tells the producer/consumer what state the slot is in.
            std::atomic<size_t> sequence; // state counter
            alignas(T) unsigned char storage[sizeof(T)];

            // std::launder is required here because the same storage
            // bytes are reused for many unrelated objects' lifetimes
            // (placement-new / destructor, over and over) as the queue
            // cycles around the ring. Without it, the compiler would be
            // entitled to assume a pointer derived from an earlier
            // (now-dead) object still refers to *that* object.
            T* data()
            {
                return std::launder(reinterpret_cast<T*>(storage));
            }
        };

    public:

        explicit Queue(size_t capacity)
            // validate_capacity() runs *first*, before mask_ or buffer_
            // are computed/allocated, so an invalid capacity throws
            // before we've allocated anything
            : capacity_(validate_capacity(capacity)),
            mask_(capacity_ - 1),
            buffer_(new Cell[capacity_]), // allocate an array of capacity_ cell objects on the heap
            enqueuePos_(0),
            dequeuePos_(0)
        {
            for (size_t i = 0; i < capacity_; ++i)
            {
                buffer_[i].sequence.store(i, std::memory_order_relaxed);
            }
        }

        ~Queue()
        {
            // Precondition, same as any container's destructor: no other
            // thread may be concurrently pushing/popping this instance.
            //
            // We destroy any remaining elements directly, in place,
            //
            // [dequeuePos_, enqueuePos_) is exactly the set of global
            // positions holding live, constructed elements, *provided*
            // no push ever completed its CAS without finishing
            // construction -- which is exactly what the nothrow_constructible
            // static_assert in try_push guarantees.
            size_t first = dequeuePos_.load(std::memory_order_relaxed);
            size_t last = enqueuePos_.load(std::memory_order_relaxed);

            for (size_t pos = first; pos != last; ++pos)
            {
                buffer_[pos & mask_].data()->~T();
            }

            delete[] buffer_;
        }

        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;

        // Attempts to enqueue `value`, returning false immediately if the
        // queue is full (never blocks).
        template<typename U>
        bool try_push(U&& value)
        {
            // Why this matters: a producer claims its slot (the CAS
            // below) *before* constructing the element in it. If that
            // construction throws, the slot's sequence number is never
            // advanced to "ready to pop" -- so every consumer position
            // behind it is permanently stuck (try_pop can only proceed
            // strictly in order), and the queue is wedged forever with
            // silent data loss for everything queued after it. There is
            // no good way to roll the claim back safely for other
            // threads that may already be mid-CAS, so instead we forbid
            // the throw entirely at compile time.
            static_assert(std::is_nothrow_constructible<T, U&&>::value,
                "lf::Queue::try_push requires a non-throwing constructor: "
                "if construction could throw after this slot is claimed, "
                "the queue would be permanently stuck for every consumer "
                "waiting behind it.");

            Cell* cell;
            size_t pos = enqueuePos_.load(std::memory_order_relaxed); // current pos

            for (;;)
            {
                cell = &buffer_[pos & mask_]; // find slot

                size_t seq = cell->sequence.load(std::memory_order_acquire); // read "dial"

                intptr_t diff =
                    static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

                if (diff == 0)
                {
                    // This slot is ready for a push at exactly this
                    // position. Try to claim it; on failure 'pos' is
                    // updated to the current value by compare_exchange_weak
                    // itself, so we just loop and re-check.
                    // Change enqueuePos_ from pos to pos + 1,
                    // but only if nobody changed it since I looked.
                    if (enqueuePos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed))
                    {
                        break;
                    }
                }
                else if (diff < 0)
                {
                    return false; // queue is full
                }
                else
                {
                    // Another producer claimed this position first;
                    // re-read and try again at the new position.
                    pos = enqueuePos_.load(std::memory_order_relaxed);
                }
            }

            new (cell->data()) T(std::forward<U>(value));

            // release: publishes both the constructed object and
            // everything the constructor wrote, to whichever consumer
            // acquires this sequence value next.
            cell->sequence.store(pos + 1, std::memory_order_release);

            return true;
        }

        // Attempts to dequeue into `result`, returning false immediately
        // if the queue is empty (never blocks).
        bool try_pop(T& result)
        {
            // Same reasoning as try_push: the slot is released (marked
            // reusable) only *after* this move-assignment completes. A
            // throwing move-assignment here would leak the source object
            // (its destructor is skipped below) and permanently strand
            // this slot, since neither producers nor consumers would
            // ever revisit it correctly. Forbid at compile time instead.
            static_assert(std::is_nothrow_move_assignable<T>::value,
                "lf::Queue::try_pop requires a non-throwing move assignment "
                "operator: if it could throw, this slot's element would "
                "leak and the slot would be permanently stranded.");
            static_assert(std::is_nothrow_destructible<T>::value,
                "lf::Queue requires a non-throwing destructor for T.");

            Cell* cell;
            size_t pos = dequeuePos_.load(std::memory_order_relaxed);

            for (;;)
            {
                cell = &buffer_[pos & mask_];

                size_t seq = cell->sequence.load(std::memory_order_acquire);

                intptr_t diff = static_cast<intptr_t>(seq) -
                    static_cast<intptr_t>(pos + 1);

                if (diff == 0)
                {
                    if (dequeuePos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed))
                    {
                        break;
                    }
                }
                else if (diff < 0)
                {
                    return false; // queue is empty
                }
                else
                {
                    pos = dequeuePos_.load(std::memory_order_relaxed);
                }
            }

            T* ptr = cell->data();
            result = std::move(*ptr);
            ptr->~T();

            // release + "pos + capacity_": marks this slot ready for the
            // push that will occur on the *next* lap through this index
            // (global position pos + capacity_), not the current one.
            cell->sequence.store(pos + capacity_, std::memory_order_release);

            return true;
        }

        size_t capacity() const noexcept
        {
            return capacity_;
        }

        // ---- Advisory statistics -------------------------------------
        // Unlike capacity(), which is fixed for the object's lifetime,
        // everything below is a momentary snapshot that can be stale
        // before the caller even reads the return value -- other threads
        // may be pushing/popping concurrently. Fine for logging/metrics/
        // dashboards; never use these to decide whether a push or pop
        // will succeed (call try_push/try_pop and check its own result
        // for that).

        size_t approximate_size() const noexcept
        {
            // enqueuePos_ is always >= dequeuePos_ in the mathematical
            // (unwrapped) sense, and unsigned subtraction gives the
            // right answer even if either counter has wrapped around
            // size_t, as long as the true difference fits in size_t.
            size_t enq = enqueuePos_.load(std::memory_order_acquire);
            size_t deq = dequeuePos_.load(std::memory_order_acquire);
            return enq - deq;
        }

        bool approximate_empty() const noexcept
        {
            return approximate_size() == 0;
        }

        bool approximate_full() const noexcept
        {
            return approximate_size() >= capacity_;
        }

        double utilization() const noexcept
        {
            return 100.0 * static_cast<double>(approximate_size()) /
                static_cast<double>(capacity_);
        }

    private:

        // Runs before any member is allocated. Rejecting capacity == 0
        // matters on its own: 0 satisfies the naive "(c & (c-1)) == 0"
        // power-of-two test, so without this explicit check a capacity
        // of 0 would slip through, mask_ would underflow to SIZE_MAX,
        // and the very first push/pop would index far out of bounds.
        // Capacity must be power of 2! becuase it uses a very fast way to wrap around the ring buffer
        // pos & mask instead of the normal modulo operation pos % capacity_
        static size_t validate_capacity(size_t capacity)
        {
            if (capacity < 2 || (capacity & (capacity - 1)) != 0)
            {
                throw std::invalid_argument(
                    "lf::Queue capacity must be a non-zero power of two");
            }
            return capacity;
        }

        const size_t capacity_;
        const size_t mask_;

        // ptr to the first Cell in an array
        Cell* buffer_; // store collection of cell objects

        // Producer and consumer counters are padded to their own cache
        // line (typically 64 bytes) so that heavy contention on one
        // doesn't cause false-sharing invalidation traffic on the other.
        alignas(64) std::atomic<size_t> enqueuePos_; // producers
        alignas(64) std::atomic<size_t> dequeuePos_; // consumers
    };

}
