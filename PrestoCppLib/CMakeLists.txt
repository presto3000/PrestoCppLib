cmake_minimum_required(VERSION 3.16)
project(PrestoCppLib LANGUAGES CXX)

# -------------------------
# Global C++ settings
# -------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

# -------------------------
# Header-only library
# -------------------------
add_library(presto INTERFACE)

target_include_directories(presto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(presto INTERFACE cxx_std_20)

target_link_libraries(presto INTERFACE OpenSSL::SSL OpenSSL::Crypto)

# -------------------------
# App
# -------------------------
add_executable(PrestoApp PrestoCppLib.cpp)
target_link_libraries(PrestoApp PRIVATE presto)

# -------------------------
# Benchmarks
# -------------------------
add_executable(presto_bench
    benchmarks/bench_main.cpp
)

target_link_libraries(presto_bench PRIVATE presto)

# -------------------------
# Tests
# -------------------------
add_executable(presto_tests 
    tests/test_main.cpp
    tests/test_lf_queue_mpmc.cpp
)
target_link_libraries(presto_tests PRIVATE presto)

# -------------------------
# Examples
# -------------------------
add_executable(presto_examples 
    examples/example_main.cpp
    examples/examples.cpp
)
target_link_libraries(presto_examples PRIVATE presto)