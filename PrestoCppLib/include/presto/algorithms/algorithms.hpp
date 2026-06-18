#pragma once

#include "presto/vector.hpp"
#include "presto/unordered_map.hpp"

/*
*   Find all duplicates in the given vector
    Each duplicate should only be included once in the result.
*/

template <typename T, typename Hash = std::hash<T>>
PrestoVector<T> findDuplicates(const PrestoVector<T>& nums)
{
    PrestoVector<T> result;
    PrestoUnordered_map<T, int, Hash> freq;

    for (int i = 0; i < nums.size(); i++) {
        int& count = freq[nums[i]];
        if (++count == 2) {
            result.push_back(nums[i]);
        }
    }

    return result;
}