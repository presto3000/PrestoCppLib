#include "examples.h"
#include <string>
#include <presto/hashing/file_hash.hpp>
#include <presto/concurrency/thread_pool.hpp>
#include <presto/vector.hpp>
#include <presto/algorithms/algorithms.hpp>
#include <presto/string.hpp>

void Example::RunFileHash()
{
	std::string hash = hashing::sha256File("TestFile1.txt");
	std::string hash2 = hashing::sha256File("TestFile1_copy.txt");
	std::cout << "SHA-256: " << hash << "\n";
	std::cout << "SHA-256 (copy): " << hash2 << "\n";
}

void Example::RunThreadPool1()
{
    presto::concurrency::ThreadPool pool(4);

    std::vector<std::future<int>> results;

    // Submit
    for (int i = 0; i < 10; i++) {
        results.push_back(pool.enqueue([](int x) {
            return x * x;
        }, i));
    }

    // Collect
    for (auto& f : results) {
        std::cout << "Result: " << f.get() << "\n";
    }
}

void Example::RunDuplicatesExample()
{
    PrestoVector<int> nums;
    nums.push_back(1);
    nums.push_back(2);
    nums.push_back(2);
    nums.push_back(3);
    nums.push_back(1);

    PrestoVector<int> dups = findDuplicates(nums);

	std::cout << "Duplicates:\n";
    for (int num : dups) {
        std::cout << num << "\n";
	}

	PrestoVector<PrestoString> str_nums = { "apple", "banana", "apple", "orange", "banana", "grape" };
	PrestoVector<PrestoString> str_dups = findDuplicates<PrestoString, PrestoStringHash>(str_nums);

	std::cout << "String Duplicates:\n";
    for (const auto& str : str_dups) {
        std::cout << str.c_str() << "\n";
	}


}
