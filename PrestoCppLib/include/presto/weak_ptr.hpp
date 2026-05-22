#pragma once

#include <atomic>
#include "shared_ptr.hpp"

template <typename T>
class PrestoWeakPtr {
private:
    ControlBlock<T>* ctrl = nullptr;

    void release() noexcept {
        if (!ctrl) return;

        ControlBlock<T>* c = ctrl;
        ctrl = nullptr;

        // Last weak reference?
        if (c->weak.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (c->strong.load(std::memory_order_acquire) == 0) {
                ::operator delete(c);
            }
        }
    }

    template <typename U>
    friend class PrestoSharedPtr;

public:
    constexpr PrestoWeakPtr() noexcept = default;

    PrestoWeakPtr(const PrestoSharedPtr<T>& sp) noexcept : ctrl(sp.ctrl) {
        if (ctrl)
        {
            ctrl->weak.fetch_add(1, std::memory_order_relaxed);
        }
    }

    PrestoWeakPtr(const PrestoWeakPtr& other) noexcept : ctrl(other.ctrl) {
        if (ctrl)
        {
            ctrl->weak.fetch_add(1, std::memory_order_relaxed);
        }
    }

    PrestoWeakPtr(PrestoWeakPtr&& other) noexcept : ctrl(other.ctrl) {
        other.ctrl = nullptr;
    }

    PrestoWeakPtr& operator=(const PrestoWeakPtr& other) noexcept {
        PrestoWeakPtr tmp(other);
        swap(tmp);
        return *this;
    }

    PrestoWeakPtr& operator=(PrestoWeakPtr&& other) noexcept {
        PrestoWeakPtr tmp(std::move(other));
        swap(tmp);
        return *this;
    }
    // from shared ptr
    PrestoWeakPtr& operator=(const PrestoSharedPtr<T>& sp) noexcept {
        PrestoWeakPtr tmp(sp);
        swap(tmp);
        return *this;
    }

    ~PrestoWeakPtr() {
        release();
    }
    // 
    void reset() noexcept { release(); }

    void swap(PrestoWeakPtr& other) noexcept {
        std::swap(ctrl, other.ctrl);
    }

    bool expired() const {
        return !ctrl || ctrl->strong.load(std::memory_order_acquire) == 0;
    }

    size_t use_count() const noexcept {
        return ctrl ? ctrl->strong.load(std::memory_order_acquire) : 0;
    }

    // Lock: atomically promote to a shared_ptr, or return empty if expired.
    // Uses a CAS loop so the strong count can never be bumped from 0.
    PrestoSharedPtr<T> lock() const noexcept {
        ControlBlock<T>* c = ctrl;
        if (!c) return {};

        for (;;) {
            size_t old = c->strong.load(std::memory_order_acquire);
            if (old == 0) return {};

            if (c->strong.compare_exchange_strong(
                old, old + 1,
                std::memory_order_acq_rel,
                std::memory_order_acquire))
            {
                return PrestoSharedPtr<T>(c);
            }
        }
    }
};

// Non-member swap
template <typename T>
void swap(PrestoWeakPtr<T>& a, PrestoWeakPtr<T>& b) noexcept {
    a.swap(b);
}
