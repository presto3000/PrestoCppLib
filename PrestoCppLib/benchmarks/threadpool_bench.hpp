#pragma once

#include <iostream>
#include <vector>
#include <future>
#include <chrono>

#include "presto/concurrency/thread_pool.hpp"
#include "bench_utils.h"

// Test 1: basic enqueue throughput
inline void benchmark_threadpool_basic() {
    const int N = 100000;
    presto::concurrency::ThreadPool pool(4);

    long long time = measure([&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < N; i++) {
            results.push_back(pool.enqueue([](int x) 
                {
                    return x + 1;
                }, i));
        }

        for (auto& r : results) 
        {
            r.get();
        }
        });

    std::cout << "ThreadPool basic:\n";
    std::cout << time << " us\n\n";
}

// Test 2: heavy parallel workload
inline void benchmark_threadpool_heavy() {
    const int N = 20000;
    presto::concurrency::ThreadPool pool(8);

    long long time = measure([&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < N; i++) {
            results.push_back(pool.enqueue([](int x) {
                for (int i = 0; i < 1000; i++) {}
                return x * 2;
                }, i));
        }

        for (auto& r : results) {
            r.get();
        }
        });

    std::cout << "ThreadPool heavy:\n";
    std::cout << time << " us\n\n";
}

inline void benchmark_std_async() {
    const int N = 20000;

    long long time = measure([&]() {
        std::vector<std::future<int>> results;

        for (int i = 0; i < N; i++) {
            results.push_back(std::async(std::launch::async, [](int x) {
                for (int i = 0; i < 1000; i++) {}
                return x * 2;
                }, i));
        }

        for (auto& r : results) {
            r.get();
        }
        });

    std::cout << "std::async:\n";
    std::cout << time << " us\n\n";
}

inline void benchmark_sequential() {
    const int N = 20000;

    long long time = measure([&]() {
        long long sum = 0;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 1000; j++) {}
            sum += i * 2;
        }

        std::cout << "Sum: " << sum << "\n";
        });

    std::cout << "sequential:\n";
    std::cout << time << " us\n\n";
}

// Test 3: small task overhead test
inline void benchmark_threadpool_overhead() {
    const int N = 50000;
    presto::concurrency::ThreadPool pool(4);

    long long time = measure([&]() {
        std::vector<std::future<void>> results;

        for (int i = 0; i < N; i++) {
            results.push_back(pool.enqueue([] {
                // empty task
                }));
        }

        for (auto& r : results) {
            r.get();
        }
        });

    std::cout << "ThreadPool overhead:\n";
    std::cout << time << " us\n\n";
}

inline double benchmark_sequential2() {
    const int N = 100'000'000;

    double result = 0;

    long long time = measure([&]() {
        for (int i = 0; i < N; i++) {
            result += std::sin(i) * std::cos(i) + std::sqrt(i + 1);
        }
        });

    std::cout << "Sequential:\n" << time << " us\n\n";

    return result;
}

inline double benchmark_threadpool2() {
    const int N = 100'000'000;
    const int threads = std::thread::hardware_concurrency();

    presto::concurrency::ThreadPool pool(threads);

    int chunk = N / threads;

    double result = 0.0;

    long long time = measure([&]() {

        std::vector<std::future<double>> futures;

        for (int t = 0; t < threads; t++) {
            int start = t * chunk;
            int end = (t == threads - 1) ? N : start + chunk;

            futures.push_back(pool.enqueue([=]() {
                double sum = 0.0;

                for (int i = start; i < end; i++) {
                    sum += std::sin(i) * std::cos(i) + std::sqrt(i + 1);
                }

                return sum;
                }));
        }

        double total = 0.0;

        for (auto& f : futures) {
            total += f.get();
        }

        result = total;
        });

    std::cout << "ThreadPool:\n" << time << " us\n\n";

    return result;
}