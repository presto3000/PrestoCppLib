#pragma once
#include <atomic>

template <typename T>
struct ControlBlock {
    std::atomic<size_t> strong{ 1 };
    std::atomic<size_t> weak{ 1 };

    alignas(T) unsigned char storage[sizeof(T)];

    template <typename... Args>
    explicit ControlBlock(Args&&... args) {
        ::new (static_cast<void*>(storage)) T(std::forward<Args>(args)...);
    }

    T* ptr() noexcept
    {
        return reinterpret_cast<T*>(storage);
    }

    // Non-copyable, non-movable
    ControlBlock(const ControlBlock&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;
};

// forward
template <typename T>
class PrestoWeakPtr;

template <typename T>
class PrestoSharedPtr {
private:
    ControlBlock<T>* ctrl = nullptr;

    void release() {
        if (!ctrl) return;

        ControlBlock<T>* c = ctrl;
        ctrl = nullptr;

        // Last strong reference?
        if (c->strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // Destroy the managed object.
            c->ptr()->~T();

            // Drop the implicit weak reference.
            if (c->weak.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                ::operator delete(c);
            }
        }
    }

    // Takes ownership of an already-incremented control block.
    explicit PrestoSharedPtr(ControlBlock<T>* c) noexcept : ctrl(c) {}

    template <typename U>
    friend class PrestoWeakPtr;

    template <typename U, typename... Args>
    friend PrestoSharedPtr<U> make_presto_shared(Args&&... args);

public:
    PrestoSharedPtr() = default;
    constexpr PrestoSharedPtr(std::nullptr_t) noexcept {}

    // copy
    PrestoSharedPtr(const PrestoSharedPtr& other) : ctrl(other.ctrl) {
        if (ctrl) {
            ctrl->strong.fetch_add(1, std::memory_order_relaxed);
        }
    }

    PrestoSharedPtr(PrestoSharedPtr&& other) noexcept : ctrl(other.ctrl) {
        other.ctrl = nullptr;
    }

    PrestoSharedPtr& operator=(const PrestoSharedPtr& other) noexcept {
        // Copy and swap: safe, handles self-assignment automatically.
        PrestoSharedPtr tmp(other);
        swap(tmp);
        return *this;
    }

    PrestoSharedPtr& operator=(PrestoSharedPtr&& other) noexcept {
        PrestoSharedPtr tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    PrestoSharedPtr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~PrestoSharedPtr() {
        release();
    }

    void reset() noexcept {
        release();
    }

    void swap(PrestoSharedPtr& other) noexcept {
        std::swap(ctrl, other.ctrl);
    }

    T* get() const noexcept {
        return ctrl ? ctrl->ptr() : nullptr;
    }

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }

    size_t use_count() const {
        return ctrl ? ctrl->strong.load(std::memory_order_acquire) : 0;
    }

    explicit operator bool() const noexcept { return ctrl != nullptr; }

    bool operator==(const PrestoSharedPtr& other) const noexcept {
        return get() == other.get();
    }
    bool operator!=(const PrestoSharedPtr& other) const noexcept {
        return !(*this == other);
    }
    bool operator==(std::nullptr_t) const noexcept { return !ctrl; }
    bool operator!=(std::nullptr_t) const noexcept { return  ctrl; }
};

// Non-member swap
template <typename T>
void swap(PrestoSharedPtr<T>& a, PrestoSharedPtr<T>& b) noexcept {
    a.swap(b);
}

// --- Factory ---
//
// Allocates ControlBlock<T> in one shot (object + metadata in the same
// allocation, just like std::make_shared), initialises T in-place, and
// wraps it. Exception-safe: if T's constructor throws, operator delete runs
// via the catch block before propagating.
template <typename T, typename... Args>
PrestoSharedPtr<T> make_presto_shared(Args&&... args) {
    void* mem = ::operator new(sizeof(ControlBlock<T>));
    ControlBlock<T>* ctrl = nullptr;
    try {
        ctrl = ::new (mem) ControlBlock<T>(std::forward<Args>(args)...);
    }
    catch (...) {
        ::operator delete(mem);
        throw;
    }
    return PrestoSharedPtr<T>(ctrl);
}