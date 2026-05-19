#pragma once

#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <cstddef>
#include "presto/vector.hpp"



template <typename Key, typename Value, typename Hash = std::hash<Key>>
class PrestoUnordered_map {

public:

    explicit PrestoUnordered_map(
        std::size_t bucket_count = 16,
        float max_load_factor = 0.75f,
        Hash hash = Hash()
    ) : buckets(bucket_count),
        num_elements(0),
        max_load_factor(max_load_factor),
        hasher(hash) {
    }

    // Insert or update value
    void insert_or_assign(const Key& key, const Value& value) {
        auto& bucket = buckets[get_index(key)];

        for (auto& kv : bucket) {
            if (kv.key == key) {
                kv.value = value;
                return;
            }
        }

        bucket.push_back({ key, value });
        ++num_elements;
        ensure_load_factor();
    }

    Value* find(const Key& key) {
        auto& bucket = buckets[get_index(key)];

        for (auto& kv : bucket) {
            if (kv.key == key) {
                return &kv.value;
            }
        }
        return nullptr;
    };


private:

    struct KeyValuePair {
        Key key;
        Value value;
    };

    using Bucket = PrestoVector<KeyValuePair>;
    // Vector of buckets for separate chaining, each bucket is a list of key-value pairs to handle collisions.
    PrestoVector<Bucket> buckets;
    std::size_t num_elements;
    float max_load_factor;
    Hash hasher;

    std::size_t get_index(const Key& key) const {
        return hasher(key) % buckets.size();
    }

    void ensure_load_factor() {
        if (buckets.empty()) return;
        float load = static_cast<float>(num_elements) / buckets.size();
        if (load > max_load_factor)
        {
            rehash(buckets.size() * 2);
        }
    }

    void rehash(std::size_t new_bucket_count)
    {
        // bigger bucket count means less collisions, but more memory usage.
        // double the bucket count to maintain O(1) average complexity.
        PrestoVector<Bucket> new_buckets(new_bucket_count);

        for (auto& bucket : buckets) {
            for (auto& kv : bucket)
            {
                std::size_t new_index = hasher(kv.key) % new_bucket_count;
                new_buckets[new_index].push_back(std::move(kv));
            }
        }

        buckets = std::move(new_buckets);
    }
};