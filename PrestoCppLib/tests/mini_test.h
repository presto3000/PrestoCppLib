#pragma once

// ============================================================================
// mini_test.hpp -- a tiny, dependency-free unit test framework.
//
// this is just a minimal CHECK()-based
// harness so test files can compile standalone with nothing but a C++17
// compiler -- no GoogleTest/Catch2/vcpkg/CMake dependency required.
//
// Usage:
//
//   #include "mini_test.hpp"
//
//   TEST(adds_two_numbers)
//   {
//       CHECK_EQ(2 + 2, 4);
//   }
//
//   int main()
//   {
//       RUN_TEST(adds_two_numbers);
//       return mini_test::summary();
//   }
//
// CHECK(cond)      -- fails (records + prints) if cond is false, keeps going.
// CHECK_EQ(a, b)   -- fails (records + prints) if a != b, keeps going.
// TEST(name)       -- declares a test function named 'name' taking no args.
// RUN_TEST(name)   -- runs a TEST()-declared function, printing its name and
//                     wiring up mini_test::g_current_test for failure output.
// mini_test::summary() -- prints a final pass/fail tally and returns a
//                          process exit code (0 = all passed, 1 = otherwise),
//                          suitable for 'return mini_test::summary();' in main.
//
// A CHECK failure never stops the enclosing test or process -- it just
// records the failure and lets the rest of the test (and suite) continue,
// so one bad assertion doesn't hide information about the rest.
// ============================================================================

#include <iostream>
#include <string>

namespace mini_test
{
    inline int& checks_run()
    {
        static int value = 0;
        return value;
    }

    inline int& checks_failed()
    {
        static int value = 0;
        return value;
    }

    inline std::string& current_test()
    {
        static std::string name;
        return name;
    }

    inline void check(bool cond, const char* expr, const char* file, int line)
    {
        ++checks_run();
        if (!cond)
        {
            ++checks_failed();
            std::cerr << "  [FAIL] " << current_test() << ": " << expr
                << "  (" << file << ":" << line << ")\n";
        }
    }

    template<typename A, typename B>
    void check_eq(const A& a, const B& b, const char* expr_a, const char* expr_b,
        const char* file, int line)
    {
        ++checks_run();
        if (!(a == b))
        {
            ++checks_failed();
            std::cerr << "  [FAIL] " << current_test() << ": " << expr_a
                << " == " << expr_b << "   (got " << a << " vs " << b
                << ")  (" << file << ":" << line << ")\n";
        }
    }

    inline void run_test(const char* name, void (*fn)())
    {
        current_test() = name;

        int failed_before = checks_failed();

        std::cout << "Running " << name << "..." << std::flush;;
        fn();

        if (checks_failed() == failed_before)
        {
            std::cout << "\033[32mPASS\033[0m\n";
        }
        else
        {
            std::cout << "\033[31mFAIL\033[0m\n";
        }
    }

    // Prints the final tally and returns a process exit code:
    // 0 if every check passed, 1 if any failed.
    inline int summary()
    {
        std::cout << "\n----------------------------------------\n";
        std::cout << checks_run() << " checks run, "
            << checks_failed() << " failed.\n";

        if (checks_failed() == 0)
        {
            std::cout << "ALL TESTS PASSED\n";
            return 0;
        }
        std::cout << "SOME TESTS FAILED\n";
        return 1;
    }
}

#define CHECK(cond) ::mini_test::check((cond), #cond, __FILE__, __LINE__)
#define CHECK_EQ(a, b) ::mini_test::check_eq((a), (b), #a, #b, __FILE__, __LINE__)
#define TEST(name) void name()
#define RUN_TEST(name) ::mini_test::run_test(#name, name)