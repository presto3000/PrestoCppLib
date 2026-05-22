
#include <vector>
#include <iostream>
#include "presto/shared_ptr.hpp"
#include "presto/weak_ptr.hpp"
#include "bench_utils.h"

// ----------------------------------------------------------------------------
// 1. make_shared  -  allocation + construction cost
// ----------------------------------------------------------------------------
inline void benchmark_make_shared() {
    const int N = 500'000;

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto sp = std::make_shared<int>(i);
            (void)sp;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto sp = make_presto_shared<int>(i);
            (void)sp;
        }
        });

    print_result("make_shared (alloc + construct + destroy)", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 2. Copy  -  ref-count increment/decrement under a hot loop
// ----------------------------------------------------------------------------
inline void benchmark_shared_copy() {
    const int N = 2'000'000;

    auto std_sp = std::make_shared<int>(42);
    auto presto_sp = make_presto_shared<int>(42);

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto copy = std_sp;
            (void)copy;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto copy = presto_sp;
            (void)copy;
        }
        });

    print_result("shared_ptr copy (refcount inc+dec x2M)", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 3. Move  -  should be near-zero cost; shows overhead of bookkeeping
// ----------------------------------------------------------------------------
inline void benchmark_shared_move() {
    const int N = 2'000'000;

    long long std_time = measure([&]() {
        auto sp = std::make_shared<int>(1);
        for (int i = 0; i < N; i++) {
            auto sp2 = std::move(sp);
            sp = std::move(sp2);
        }
        });

    long long presto_time = measure([&]() {
        auto sp = make_presto_shared<int>(1);
        for (int i = 0; i < N; i++) {
            auto sp2 = std::move(sp);
            sp = std::move(sp2);
        }
        });

    print_result("shared_ptr move (ping-pong x2M)", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 4. Dereference  -  raw pointer access through the wrapper
// ----------------------------------------------------------------------------
inline void benchmark_dereference() {
    const int N = 5'000'000;

    auto std_sp = std::make_shared<int>(0);
    auto presto_sp = make_presto_shared<int>(0);

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile int x = *std_sp;
            (void)x;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile int x = *presto_sp;
            (void)x;
        }
        });

    print_result("dereference (*sp) x5M", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 5. Weak lock (hit)  -  lock() when the object is still alive
// ----------------------------------------------------------------------------
inline void benchmark_weak_lock_hit() {
    const int N = 1'000'000;

    auto std_sp = std::make_shared<int>(7);
    auto presto_sp = make_presto_shared<int>(7);

    std::weak_ptr<int>    std_wp = std_sp;
    PrestoWeakPtr<int>    presto_wp = presto_sp;

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto locked = std_wp.lock();
            (void)locked;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto locked = presto_wp.lock();
            (void)locked;
        }
        });

    print_result("weak_ptr::lock() - live object (hit) x1M", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 6. Weak lock (miss)  -  lock() when the object is already gone
// ----------------------------------------------------------------------------
inline void benchmark_weak_lock_miss() {
    const int N = 1'000'000;

    std::weak_ptr<int>  std_wp;
    PrestoWeakPtr<int>  presto_wp;

    {
        auto std_sp = std::make_shared<int>(7);
        auto presto_sp = make_presto_shared<int>(7);
        std_wp = std_sp;
        presto_wp = presto_sp;
    }   // both objects destroyed here

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto locked = std_wp.lock();
            (void)locked;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto locked = presto_wp.lock();
            (void)locked;
        }
        });

    print_result("weak_ptr::lock() - expired object (miss) x1M", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 7. Weak copy  -  weak refcount inc/dec
// ----------------------------------------------------------------------------
inline void benchmark_weak_copy() {
    const int N = 2'000'000;

    auto std_sp = std::make_shared<int>(1);
    auto presto_sp = make_presto_shared<int>(1);

    std::weak_ptr<int>  std_wp = std_sp;
    PrestoWeakPtr<int>  presto_wp = presto_sp;

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto copy = std_wp;
            (void)copy;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto copy = presto_wp;
            (void)copy;
        }
        });

    print_result("weak_ptr copy (weak refcount inc+dec x2M)", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 8. Reset chain  -  repeated reset() + reassign (exercises full release path)
// ----------------------------------------------------------------------------
inline void benchmark_reset_reassign() {
    const int N = 500'000;

    long long std_time = measure([&]() {
        std::shared_ptr<int> sp;
        for (int i = 0; i < N; i++) {
            sp = std::make_shared<int>(i);
            sp.reset();
        }
        });

    long long presto_time = measure([&]() {
        PrestoSharedPtr<int> sp;
        for (int i = 0; i < N; i++) {
            sp = make_presto_shared<int>(i);
            sp.reset();
        }
        });

    print_result("reset + reassign cycle x500K", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 9. Vector of shared_ptrs  -  realistic container ownership pattern
// ----------------------------------------------------------------------------
inline void benchmark_vector_of_shared() {
    const int N = 200'000;

    long long std_time = measure([&]() {
        std::vector<std::shared_ptr<int>> v;
        v.reserve(N);
        for (int i = 0; i < N; i++)
            v.push_back(std::make_shared<int>(i));
        });  // vector destructor runs refcounts down

    long long presto_time = measure([&]() {
        std::vector<PrestoSharedPtr<int>> v;
        v.reserve(N);
        for (int i = 0; i < N; i++)
            v.push_back(make_presto_shared<int>(i));
        });

    print_result("vector<shared_ptr<int>> fill + destroy x200K", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// 10. Cycle creation + teardown
// ----------------------------------------------------------------------------
struct BenchNode {
    PrestoSharedPtr<BenchNode> next;
    PrestoWeakPtr<BenchNode>   prev;
};

struct StdBenchNode {
    std::shared_ptr<StdBenchNode> next;
    std::weak_ptr<StdBenchNode>   prev;
};

inline void benchmark_cycle() {
    const int N = 200'000;

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto a = std::make_shared<StdBenchNode>();
            auto b = std::make_shared<StdBenchNode>();
            a->next = b;
            b->prev = a;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            auto a = make_presto_shared<BenchNode>();
            auto b = make_presto_shared<BenchNode>();
            a->next = b;
            b->prev = a;
        }
        });

    print_result("cycle create + teardown (shared+weak pair) x200K", std_time, presto_time);
}

// ----------------------------------------------------------------------------
// Run all
// ----------------------------------------------------------------------------
inline void run_presto_ptr_benchmarks() {
    std::cout << "========================================\n";
    std::cout << "  PrestoPtr vs std - benchmarks\n";
    std::cout << "========================================\n\n";

    benchmark_make_shared();
    benchmark_shared_copy();
    benchmark_shared_move();
    benchmark_dereference();
    benchmark_weak_lock_hit();
    benchmark_weak_lock_miss();
    benchmark_weak_copy();
    benchmark_reset_reassign();
    benchmark_vector_of_shared();
    benchmark_cycle();

    std::cout << "========================================\n";
    std::cout << "  Done.\n";
    std::cout << "========================================\n";
}