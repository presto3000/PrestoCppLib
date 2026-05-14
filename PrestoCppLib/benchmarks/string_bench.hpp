#pragma once

#include <iostream>
#include <string>

#include "presto/string.hpp"
#include "bench_utils.h"

// Test1: repeated append
inline void benchmark_append() {
    const int N = 100000;

    long long std_time = measure([&]() {
        std::string s;
        for (int i = 0; i < N; i++) {
            s += "a";
        }
        });

    long long presto_time = measure([&]() {
        PrestoString s;
        for (int i = 0; i < N; i++) {
            s.append("a");
        }
        });

    std::cout << "Append test:\n";
    std::cout << "std::string: " << std_time << " us\n";
    std::cout << "PrestoString: " << presto_time << " us\n\n";
}
// Test 2: operator+
inline void benchmark_concat() {
    const int N = 5000;

    long long std_time = measure([&]() {
        std::string s = "a";
        for (int i = 0; i < N; i++) {
            s = s + "b";
        }
        });

    long long presto_time = measure([&]() {
        PrestoString s("a");
        for (int i = 0; i < N; i++) {
            s = s + "b";
        }
        });

    std::cout << "Concatenation test:\n";
    std::cout << "std::string: " << std_time << " us\n";
    std::cout << "PrestoString: " << presto_time << " us\n\n";
}

// Test 3: copy behavior
inline void benchmark_copy()
{
    PrestoString base("HelloWorldHelloWorldHelloWorld");

    const int N = 1000000;

    long long std_time = measure([&]() {
        std::string s = "HelloWorldHelloWorldHelloWorld";
        for (int i = 0; i < N; i++) {
            std::string copy = s;
        }
        });

    long long presto_time = measure([&]() {
        PrestoString s = base;
        for (int i = 0; i < N; i++) {
            PrestoString copy = s;
        }
        });

    std::cout << "Copy test:\n";
    std::cout << "std::string: " << std_time << " us\n";
    std::cout << "PrestoString: " << presto_time << " us\n\n";
}

// Test 4: mixed workload
inline void benchmark_mixed() {
    const int N = 20000;

    long long std_time = measure([&]() {
        std::string s;
        for (int i = 0; i < N; i++) {
            s += "abc";
            s += "def";
            std::string copy = s;
        }
        });

    long long presto_time = measure([&]() {
        PrestoString s;
        for (int i = 0; i < N; i++) {
            s.append("abc");
            s.append("def");
            PrestoString copy = s;
        }
        });

    std::cout << "Mixed workload:\n";
    std::cout << "std::string: " << std_time << " us\n";
    std::cout << "PrestoString: " << presto_time << " us\n\n";
}
