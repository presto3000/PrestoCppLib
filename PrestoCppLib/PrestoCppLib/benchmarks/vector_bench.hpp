#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include "presto/vector.hpp"


using Clock = std::chrono::high_resolution_clock;

template <typename Func>
long long measure(Func f) {
    auto start = Clock::now();
    f();
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

// Presto Vector -------------------------------------------------------------------------------------------------
// Test 1: push_back(growth behavior)
inline void benchmark_push_back() {
    const int N = 1000000;

    long long std_time = measure([&]() {
        std::vector<int> v;
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        });

    long long presto_time = measure([&]() {
        PrestoVector<int> v;
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        });

    std::cout << "Push_back test:\n";
    std::cout << "std::vector: " << std_time << " us\n";
    std::cout << "PrestoVector: " << presto_time << " us\n\n";
}

// Test 2: reserve + push_back (no reallocations)
inline void benchmark_reserve_push() {
    const int N = 1000000;

    long long std_time = measure([&]() {
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        });

    long long presto_time = measure([&]() {
        PrestoVector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        });

    std::cout << "Reserve + push_back test:\n";
    std::cout << "std::vector: " << std_time << " us\n";
    std::cout << "PrestoVector: " << presto_time << " us\n\n";
}

// Test 3: copy behavior
inline void benchmark_copy_vector() {
    const int N = 200000;

    std::vector<int> std_base(N, 42);
    PrestoVector<int> presto_base;
    presto_base.reserve(N);
    for (int i = 0; i < N; i++) {
        presto_base.push_back(42);
    }

    long long std_time = measure([&]() {
        for (int i = 0; i < 100; i++) {
            std::vector<int> copy = std_base;
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < 100; i++) {
            PrestoVector<int> copy = presto_base;
        }
        });

    std::cout << "Copy test:\n";
    std::cout << "std::vector: " << std_time << " us\n";
    std::cout << "PrestoVector: " << presto_time << " us\n\n";
}

// Test 4: pop_back performance
inline void benchmark_pop_back() {
    const int N = 1000000;

    long long std_time = measure([&]() {
        std::vector<int> v(N, 1);
        for (int i = 0; i < N; i++) {
            v.pop_back();
        }
        });

    long long presto_time = measure([&]() {
        PrestoVector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; i++) v.push_back(1);

        for (int i = 0; i < N; i++) {
            v.pop_back();
        }
        });

    std::cout << "Pop_back test:\n";
    std::cout << "std::vector: " << std_time << " us\n";
    std::cout << "PrestoVector: " << presto_time << " us\n\n";
}

// Test 5: mixed workload (realistic usage)
inline void benchmark_vector_mixed() {
    const int N = 200000;

    long long std_time = measure([&]() {
        std::vector<int> v;
        for (int i = 0; i < N; i++) {
            v.push_back(i);
            v.push_back(i * 2);
            std::vector<int> copy = v;
        }
        });

    long long presto_time = measure([&]() {
        PrestoVector<int> v;
        for (int i = 0; i < N; i++) {
            v.push_back(i);
            v.push_back(i * 2);
            PrestoVector<int> copy = v;
        }
        });

    std::cout << "Mixed workload:\n";
    std::cout << "std::vector: " << std_time << " us\n";
    std::cout << "PrestoVector: " << presto_time << " us\n\n";
}