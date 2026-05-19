#pragma once

#include <iostream>
#include <string>

#include "bench_utils.h"
#include <presto/unordered_map.hpp>

inline void benchmark_map_insert() {
    const int N = 500000;

    long long std_time = measure([&]() {
        std::unordered_map<int, int> m;
        for (int i = 0; i < N; i++) {
            m[i] = i * 2;
        }
        });

    long long presto_time = measure([&]() {
        PrestoUnordered_map<int, int> m;
        for (int i = 0; i < N; i++) {
            m.insert_or_assign(i, i * 2);
        }
        });

    std::cout << "Map insert test:\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_find_hit() {
    const int N = 500000;

    std::unordered_map<int, int> std_map;
    PrestoUnordered_map<int, int> presto_map;

    for (int i = 0; i < N; i++) {
        std_map[i] = i;
        presto_map.insert_or_assign(i, i);
    }

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile auto it = std_map.find(i);
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile auto it = presto_map.find(i);
        }
        });

    std::cout << "Map find (all hits):\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_find_miss() {
    const int N = 500000;

    std::unordered_map<int, int> std_map;
    PrestoUnordered_map<int, int> presto_map;

    for (int i = 0; i < N; i++) {
        std_map[i] = i;
        presto_map.insert_or_assign(i, i);
    }

    long long std_time = measure([&]() {
        for (int i = N; i < N * 2; i++) {
            volatile auto it = std_map.find(i);
        }
        });

    long long presto_time = measure([&]() {
        for (int i = N; i < N * 2; i++) {
            volatile auto it = presto_map.find(i);
        }
        });

    std::cout << "Map find (all misses):\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_mixed() {
    const int N = 200000;

    long long std_time = measure([&]() {
        std::unordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            m[i] = i;
            m[i * 2] = i;
            volatile auto it = m.find(i);
        }
        });

    long long presto_time = measure([&]() {
        PrestoUnordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            m.insert_or_assign(i, i);
            m.insert_or_assign(i * 2, i);
            volatile auto it = m.find(i);
        }
        });

    std::cout << "Map mixed workload:\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_rehash() {
    const int N = 500000;

    long long presto_time = measure([&]() {
        PrestoUnordered_map<int, int> m(16, 0.75f);

        for (int i = 0; i < N; i++) {
            m.insert_or_assign(i, i);
        }
        });

    std::cout << "Presto rehash stress test:\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_operator_brackets() {
    const int N = 500000;

    long long std_time = measure([&]() {
        std::unordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            auto& x = m[i];
            x = i;
            x += 1;
        }
        });

    long long presto_time = measure([&]() {
        PrestoUnordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            auto& x = m[i];
            x = i;
            x += 1;
        }
        });

    std::cout << "operator[] test:\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_erase() {
    const int N = 300000;

    long long std_time = measure([&]() {
        std::unordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            m[i] = i;
        }

        for (int i = 0; i < N; i++) {
            m.erase(i);
        }
        });

    long long presto_time = measure([&]() {
        PrestoUnordered_map<int, int> m;

        for (int i = 0; i < N; i++) {
            m.insert_or_assign(i, i);
        }

        for (int i = 0; i < N; i++) {
            m.erase(i);
        }
        });

    std::cout << "erase test:\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}

inline void benchmark_map_contains() {
    const int N = 500000;

    std::unordered_map<int, int> std_map;
    PrestoUnordered_map<int, int> presto_map;

    for (int i = 0; i < N; i++) {
        std_map[i] = i;
        presto_map.insert_or_assign(i, i);
    }

    long long std_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile bool x = std_map.find(i) != std_map.end();
        }
        });

    long long presto_time = measure([&]() {
        for (int i = 0; i < N; i++) {
            volatile bool x = presto_map.contains(i);
        }
        });

    std::cout << "contains test:\n";
    std::cout << "std::unordered_map: " << std_time << " us\n";
    std::cout << "PrestoUnordered_map: " << presto_time << " us\n\n";
}