#include <iostream>
#include <string>

#include "presto/vector.hpp"
#include "bench_utils.h"
#include "presto/linked_list.hpp"

// 1. append (push_back equivalent)
inline void benchmark_list_append() {
    const int N = 1'000'000;

    long long std_time = measure([&]() {
        std::list<int> v;
        for (int i = 0; i < N; i++) {
            v.push_back(i);
        }
        });

    long long presto_time = measure([&]() {
        LinkedList<int> v;
        for (int i = 0; i < N; i++) {
            v.append(i);
        }
        });

    std::cout << "Append test:\n";
    std::cout << "std::list: " << std_time << " us\n";
    std::cout << "LinkedList: " << presto_time << " us\n\n";
}
// 2. prepend
inline void benchmark_list_prepend() {
    const int N = 500'000;

    long long std_time = measure([&]() {
        std::list<int> v;
        for (int i = 0; i < N; i++) {
            v.push_front(i);
        }
        });

    long long presto_time = measure([&]() {
        LinkedList<int> v;
        for (int i = 0; i < N; i++) {
            v.prepend(i);
        }
        });

    std::cout << "Prepend test:\n";
    std::cout << "std::list: " << std_time << " us\n";
    std::cout << "LinkedList: " << presto_time << " us\n\n";
}
// 3. delete from middle (deleteAt with index N/2 repeatedly)
inline void benchmark_list_delete_middle() {
    const int N = 100'000;
    const int OPS = 20'000;

    long long std_time = measure([&]() {
        std::list<int> v(N, 1);

        for (int i = 0; i < OPS && !v.empty(); ++i) {
            auto it = v.begin();
            std::advance(it, v.size() / 2);
            v.erase(it);
        }
        });

    long long presto_time = measure([&]() {
        LinkedList<int> v;

        for (int i = 0; i < N; ++i)
            v.append(1);

        for (int i = 0; i < OPS && !v.isEmpty(); ++i)
            v.erase(v.getLength() / 2);
        });

    std::cout << "Delete middle test:\n";
    std::cout << "std::list:   " << std_time << " us\n";
    std::cout << "LinkedList:  " << presto_time << " us\n\n";
}
// 4. traversal
inline void benchmark_list_traversal() {
    const int N = 1'000'000;

    std::list<int> std_list;
    LinkedList<int> presto_list;

    for (int i = 0; i < N; i++) {
        std_list.push_back(i);
        presto_list.append(i);
    }

    long long std_time = measure([&]() {
        long long sum = 0;
        for (auto& x : std_list) {
            sum += x;
        }
        });

    long long presto_time = measure([&]() {
        long long sum = 0;
        presto_list.forEach([&](const int& x) {
            sum += x;
            });
        });

    std::cout << "Traversal test:\n";
    std::cout << "std::list: " << std_time << " us\n";
    std::cout << "LinkedList: " << presto_time << " us\n\n";
}

// 5. random access (THIS will expose weakness)
inline void benchmark_list_random_access() {
    const int N = 50'000;
    const int OPS = 20'000;

    LinkedList<int> list;
    std::list<int> std_list;

    for (int i = 0; i < N; i++) {
        list.append(i);
        std_list.push_back(i);
    }

    long long presto_time = measure([&]() {
        long long sum = 0;
        for (int i = 0; i < OPS; i++) {
            sum += list.at(i % N);
        }
        });

    long long std_time = measure([&]() {
        long long sum = 0;
        for (int i = 0; i < OPS; i++) {
            auto it = std_list.begin();
            std::advance(it, i % N);
            sum += *it;
        }
        });

    std::cout << "Random access test:\n";
    std::cout << "std::list: " << std_time << " us\n";
    std::cout << "LinkedList: " << presto_time << " us\n\n";
}

inline void run_all_linked_list_benchmarks() {
    std::cout << "=============================\n";
    std::cout << " LinkedList BENCHMARK SUITE\n";
    std::cout << "=============================\n\n";

    benchmark_list_append();
    benchmark_list_prepend();
    benchmark_list_delete_middle();
    benchmark_list_traversal();
    benchmark_list_random_access();

    std::cout << "=============================\n";
    std::cout << " DONE\n";
    std::cout << "=============================\n";
}