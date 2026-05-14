#include <iostream>
#include "vector_bench.hpp"
#include "string_bench.hpp"

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

    benchmark_insert();
    benchmark_erase();


    /*
    std::cout << "=============================\n";
    std::cout << "PrestoString Benchmarks\n";
    std::cout << "=============================\n\n";

    benchmark_append();
    benchmark_concat();
    benchmark_copy();
    benchmark_mixed();
    */



    return 0;
}