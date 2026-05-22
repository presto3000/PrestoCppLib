
#pragma once

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

template <typename Func>
long long measure(Func f) {
    auto start = Clock::now();
    f();
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

inline void print_result(const char* test, long long std_us, long long presto_us) 
{
    double ratio = std_us == 0 ? 0.0 : static_cast<double>(presto_us) / std_us;
    std::cout << test << "\n";
    std::cout << "  std:    " << std_us << " us\n";
    std::cout << "  presto: " << presto_us << " us";
    std::cout << "  (" << ratio << "x)\n\n";
}