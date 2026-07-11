// ============================================================================
// Unit tests for lf::Queue
//
// Self-contained: no GoogleTest/Catch2/etc. dependency, just a tiny
// CHECK()-based micro-framework, so this compiles anywhere with a C++17

// NOTE ON WHAT ISN'T (AND CAN'T BE) TESTED HERE:
// lf::Queue uses static_assert to reject, at COMPILE TIME, any T whose
// constructor/move-assignment/destructor could throw (see lf_queue.hpp).
// A runtime test binary can't exercise "this fails to compile" -- by
// definition, if it compiled, that check didn't fire. Those guards are
// instead verified by attempting to compile deliberately-bad code and
// confirming the build fails; see the comment at the bottom of this file
// for how to do that check separately.
// ============================================================================

#include "presto/lf_queue_mpmc.hpp"
#include "test_lf_queue_mpmc.h"
#include "mini_test.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// ============================================================================
// Construction / validation
// ============================================================================

TEST(construction_accepts_valid_power_of_two_capacities)
{
    lf::Queue<int> b(2);
    lf::Queue<int> c(4);
    lf::Queue<int> d(1024);

    CHECK_EQ(b.capacity(), 2u);
    CHECK_EQ(c.capacity(), 4u);
    CHECK_EQ(d.capacity(), 1024u);
}

TEST(construction_rejects_zero_capacity)
{
    bool threw = false;
    try
    {
        lf::Queue<int> q(0);
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    CHECK(threw);
}

TEST(construction_rejects_non_power_of_two_capacities)
{
    for (size_t bad : {3u, 5u, 6u, 7u, 100u, 1023u})
    {
        bool threw = false;
        try
        {
            lf::Queue<int> q(bad);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }
        CHECK(threw);
    }
}

// ============================================================================
// Basic FIFO correctness (single-threaded)
// ============================================================================

TEST(preserves_fifo_order)
{
    lf::Queue<int> q(16);

    for (int i = 1; i <= 10; ++i)
    {
        CHECK(q.try_push(i));
    }

    for (int expected = 1; expected <= 10; ++expected)
    {
        int val = -1;
        CHECK(q.try_pop(val));
        CHECK_EQ(val, expected);
    }

    // Queue should be empty now.
    int val;
    CHECK(!q.try_pop(val));
}

TEST(pop_only_writes_result_on_success)
{
    // try_pop must not touch 'result' when it returns false -- callers
    // shouldn't rely on this either way, but it should never crash or
    // read from uninitialized memory internally regardless.
    lf::Queue<int> q(4);
    int val = 777;
    bool ok = q.try_pop(val);
    CHECK(!ok);
    CHECK_EQ(val, 777); // untouched, since queue is empty
}

// ============================================================================
// Full / empty boundary behavior
// ============================================================================

TEST(reports_full_at_capacity_and_accepts_again_after_a_pop)
{
    lf::Queue<int> q(4);

    CHECK(q.try_push(1));
    CHECK(q.try_push(2));
    CHECK(q.try_push(3));
    CHECK(q.try_push(4));

    // Now full: further pushes must fail, and must not corrupt state.
    CHECK(!q.try_push(5));
    CHECK(!q.try_push(6));
    CHECK_EQ(q.approximate_size(), 4u);
    CHECK(q.approximate_full());

    int val;
    CHECK(q.try_pop(val));
    CHECK_EQ(val, 1);

    // One slot freed up: exactly one more push should now succeed.
    CHECK(q.try_push(5));
    CHECK(!q.try_push(6));
}

TEST(reports_empty_on_a_fresh_queue)
{
    lf::Queue<int> q(8);
    CHECK(q.approximate_empty());
    int val;
    CHECK(!q.try_pop(val));
}

TEST(construction_rejects_capacity_of_one)
{
    // capacity == 1 is mathematically degenerate for this algorithm (see
    // the comment on validate_capacity in lf_queue.hpp) and must be
    // rejected, not merely discouraged.
    bool threw = false;
    try
    {
        lf::Queue<int> q(1);
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    CHECK(threw);
}

TEST(capacity_two_edge_case)
{
    // 2 is the smallest valid capacity.
    lf::Queue<int> q(2);
    CHECK(q.try_push(42));
    CHECK(q.try_push(43));
    CHECK(!q.try_push(99)); // full

    int val;
    CHECK(q.try_pop(val));
    CHECK_EQ(val, 42);
    CHECK(q.try_push(99)); // one slot freed, reusable
    CHECK(q.try_pop(val));
    CHECK_EQ(val, 43);
    CHECK(q.try_pop(val));
    CHECK_EQ(val, 99);
    CHECK(!q.try_pop(val));
}

// ============================================================================
// Ring wrap-around: push/pop repeatedly, far more times than capacity,
// to exercise the sequence-number math across many laps around the ring.
// ============================================================================

TEST(survives_many_laps_around_a_small_ring)
{
    lf::Queue<int> q(4);
    constexpr int kIterations = 100000;

    for (int i = 0; i < kIterations; ++i)
    {
        CHECK(q.try_push(i));
        int val = -1;
        CHECK(q.try_pop(val));
        CHECK_EQ(val, i);
    }

    CHECK(q.approximate_empty());
}

TEST(survives_many_laps_with_partial_fill)
{
    // Keep 2 out of 4 slots occupied at all times while cycling many
    // laps, so both "some slots ahead, some behind" states get exercised
    // repeatedly, not just the fully-drained-each-time pattern above.
    lf::Queue<int> q(4);
    constexpr int kIterations = 50000;

    CHECK(q.try_push(-1));
    CHECK(q.try_push(-2));

    for (int i = 0; i < kIterations; ++i)
    {
        CHECK(q.try_push(i));
        int val = -1;
        CHECK(q.try_pop(val));
    }

    // Drain the 2 remaining original items plus whatever is left.
    int drained = 0;
    int val;
    while (q.try_pop(val)) ++drained;
    CHECK_EQ(drained, 2);
}

// ============================================================================
// Move semantics / move-only types
// ============================================================================

struct MoveOnly
{
    int value;
    explicit MoveOnly(int v) noexcept : value(v) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = -1; }
    MoveOnly& operator=(MoveOnly&& other) noexcept
    {
        value = other.value;
        other.value = -1;
        return *this;
    }
};

TEST(supports_move_only_types)
{
    lf::Queue<MoveOnly> q(8);

    CHECK(q.try_push(MoveOnly(10)));
    CHECK(q.try_push(MoveOnly(20)));

    MoveOnly out(0);
    CHECK(q.try_pop(out));
    CHECK_EQ(out.value, 10);

    CHECK(q.try_pop(out));
    CHECK_EQ(out.value, 20);
}

TEST(moved_from_source_is_left_in_moved_from_state)
{
    lf::Queue<std::string> q(8);
    std::string s = "hello";

    CHECK(q.try_push(std::move(s)));
    // Standard library guarantees a moved-from std::string is valid but
    // unspecified; libstdc++ leaves it empty. We just check it changed.
    CHECK(s != "hello");

    std::string out;
    CHECK(q.try_pop(out));
    CHECK_EQ(out, "hello");
}

// ============================================================================
// Destructor cleans up any elements still left in the queue
// ============================================================================

struct Counted
{
    static std::atomic<int> alive;

    int value;
    explicit Counted(int v) noexcept : value(v) { ++alive; }
    Counted(const Counted& other) noexcept : value(other.value) { ++alive; }
    Counted(Counted&& other) noexcept : value(other.value) { ++alive; other.value = -1; }
    Counted& operator=(Counted&& other) noexcept
    {
        value = other.value;
        other.value = -1;
        return *this;
    }
    ~Counted() { --alive; }
};
std::atomic<int> Counted::alive{ 0 };

TEST(destructor_destroys_remaining_elements_without_leaking)
{
    CHECK_EQ(Counted::alive.load(), 0);

    {
        lf::Queue<Counted> q(8);
        CHECK(q.try_push(Counted(1)));
        CHECK(q.try_push(Counted(2)));
        CHECK(q.try_push(Counted(3)));

        // Pop only one -- two elements are deliberately left inside when
        // the queue is destroyed below.
        Counted out(0);
        CHECK(q.try_pop(out));
        // 2 elements remain in the queue, plus 'out' itself is alive
        // (holding the moved-out value) = 3 total live Counted objects.
        CHECK_EQ(Counted::alive.load(), 3);
    } // <-- q destroyed here; should destroy its remaining 2 elements

    CHECK_EQ(Counted::alive.load(), 0);
}

// ============================================================================
// Advisory statistics sanity checks
// ============================================================================

TEST(approximate_stats_track_pushes_and_pops)
{
    lf::Queue<int> q(16);

    CHECK_EQ(q.approximate_size(), 0u);
    CHECK(q.approximate_empty());
    CHECK(!q.approximate_full());

    for (int i = 0; i < 8; ++i) q.try_push(i);

    CHECK_EQ(q.approximate_size(), 8u);
    CHECK(!q.approximate_empty());
    CHECK(!q.approximate_full());
    CHECK_EQ(q.utilization(), 50.0);

    int val;
    for (int i = 0; i < 8; ++i) q.try_pop(val);

    CHECK(q.approximate_empty());
    CHECK_EQ(q.utilization(), 0.0);
}

// ============================================================================
// Multithreaded MPMC stress test: no lost, duplicated, or corrupted items
// ============================================================================

TEST(mpmc_stress_no_lost_or_duplicated_items)
{
    constexpr size_t kCapacity = 256;
    constexpr int kProducers = 4;
    constexpr int kConsumers = 4;
    constexpr int kItemsPerProducer = 20000;
    constexpr long long kTotalItems =
        static_cast<long long>(kProducers) * kItemsPerProducer;

    lf::Queue<long long> q(kCapacity);

    std::atomic<long long> produced_sum{ 0 };
    std::atomic<long long> consumed_sum{ 0 };
    std::atomic<long long> consumed_count{ 0 };
    std::atomic<bool> done_producing{ false };

    std::vector<std::thread> producers, consumers;

    for (int p = 0; p < kProducers; ++p)
    {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < kItemsPerProducer; ++i)
            {
                long long value =
                    static_cast<long long>(p) * kItemsPerProducer + i;
                while (!q.try_push(value))
                {
                    std::this_thread::yield();
                }
                produced_sum.fetch_add(value, std::memory_order_relaxed);
            }
            });
    }

    for (int c = 0; c < kConsumers; ++c)
    {
        consumers.emplace_back([&]() {
            long long v;
            for (;;)
            {
                if (q.try_pop(v))
                {
                    consumed_sum.fetch_add(v, std::memory_order_relaxed);
                    consumed_count.fetch_add(1, std::memory_order_relaxed);
                }
                else if (done_producing.load(std::memory_order_acquire))
                {
                    if (!q.try_pop(v)) break; // drain stragglers, then exit
                    consumed_sum.fetch_add(v, std::memory_order_relaxed);
                    consumed_count.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
            });
    }

    for (auto& t : producers) t.join();
    done_producing.store(true, std::memory_order_release);
    for (auto& t : consumers) t.join();

    CHECK_EQ(consumed_count.load(), kTotalItems);
    CHECK_EQ(produced_sum.load(), consumed_sum.load());
}
