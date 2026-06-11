#include <iostream>
#include "vector_bench.hpp"
#include "string_bench.hpp"
#include "unordered_map_bench.hpp"
#include "sharedweak_bench.hpp"
#include "threadpool_bench.hpp"

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
    // benchmark_map_insert();
    // benchmark_map_find_hit();
    // benchmark_map_find_miss();
    // benchmark_map_mixed();
    // benchmark_map_rehash();
    // 
    // benchmark_map_operator_brackets();
    // benchmark_map_contains();
    // benchmark_map_erase();
    
    // Shared / Weak Ptr
    // run_presto_ptr_benchmarks();

	// --- ThreadPool ---
    // benchmark_threadpool_basic();
    // benchmark_threadpool_heavy();
    // benchmark_std_async();
    // benchmark_sequential();

    double seq = benchmark_sequential2();
    double tp = benchmark_threadpool2();

    std::cout << "Sequential result: " << seq << "\n";
    std::cout << "ThreadPool result: " << tp << "\n";

    return 0;
}