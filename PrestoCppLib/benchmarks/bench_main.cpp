#include <iostream>
#include "vector_bench.hpp"
#include "string_bench.hpp"
#include "unordered_map_bench.hpp"

int main() {
    /*
    std::cout << "=============================\n";
    std::cout << "PrestoVector Benchmarks\n";
    std::cout << "=============================\n\n";

    benchmark_push_back();
    benchmark_reserve_push();
    benchmark_copy_vector();
    benchmark_pop_back();
    benchmark_vector_mixed();
    */

    // benchmark_insert();
    // benchmark_erase();


    /*
    std::cout << "=============================\n";
    std::cout << "PrestoString Benchmarks\n";
    std::cout << "=============================\n\n";

    benchmark_append();
    benchmark_concat();
    benchmark_copy();
    benchmark_mixed();
    */


    // Map
    benchmark_map_insert();
    benchmark_map_find_hit();
    benchmark_map_find_miss();
    benchmark_map_mixed();
    benchmark_map_rehash();


    return 0;
}