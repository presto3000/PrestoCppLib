
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


