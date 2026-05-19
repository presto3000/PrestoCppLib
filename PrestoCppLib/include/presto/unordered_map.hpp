#pragma once

#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <cstddef>
#include "presto/vector.hpp"

/*
* A simple implementation of an unordered map (hash table) 
    using separate chaining for collision resolution.
*/

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
    template <typename V>
    void insert_or_assign(const Key& key, V&& value) {
        // Rehash *before* inserting so no reference is ever invalidated
        if (would_exceed_load_factor())
            rehash(buckets.size() * 2);

        auto& bucket = buckets[get_index(key)];
        if (auto* kv = find_in_bucket(bucket, key)) {
            kv->value = std::forward<V>(value);
            return;
        }
        bucket.push_back({ key, std::forward<V>(value) });
        ++num_elements;
    }

    Value* find(const Key& key) {
        auto& bucket = buckets[get_index(key)];
        KeyValuePair* kv = find_in_bucket(bucket, key);  // explicit non-const type
        return kv ? &kv->value : nullptr;
    }

    const Value* find(const Key& key) const {
        const auto& bucket = buckets[get_index(key)];
        const KeyValuePair* kv = find_in_bucket(bucket, key);
        return kv ? &kv->value : nullptr;
    }

    std::size_t size()  const { return num_elements; }
    bool empty() const { return num_elements == 0; }

    bool contains(const Key& key) const {
        return find(key) != nullptr;
    }

    bool erase(const Key& key) {
        auto& bucket = buckets[get_index(key)];

        for (auto it = bucket.begin(); it != bucket.end(); ++it)
        {
            if (it->key == key) {
                bucket.erase(it);
                --num_elements;
                return true;
            }
        }
        return false;
    }

    // ref to value for given key, inserts default if not found
    Value& operator[](const Key& key) {
        if (would_exceed_load_factor())
            rehash(buckets.size() * 2);

        auto& bucket = buckets[get_index(key)];
        if (auto* kv = find_in_bucket(bucket, key))
            return kv->value;

        bucket.push_back({ key, Value{} });
        ++num_elements;
        return bucket.back().value;
    }

    // rvalue overload for operator[] to avoid unnecessary copy when key is a string or complex type
    Value& operator[](Key&& key) {
        if (would_exceed_load_factor())
            rehash(buckets.size() * 2);

        auto& bucket = buckets[get_index(key)];
        if (auto* kv = find_in_bucket(bucket, key))
            return kv->value;

        bucket.push_back({ std::move(key), Value{} });
        ++num_elements;
        return bucket.back().value;
    }

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

    // ------------------- helper functions -------------------

    std::size_t get_index(const Key& key) const {
        return hasher(key) % buckets.size();
    }


    KeyValuePair* find_in_bucket(Bucket& bucket, const Key& key) {
        for (auto& kv : bucket)
            if (kv.key == key) return &kv;
        return nullptr;
    }

    const KeyValuePair* find_in_bucket(const Bucket& bucket,
        const Key& key) const {
        for (const auto& kv : bucket)
            if (kv.key == key) return &kv;
        return nullptr;
    }

    bool would_exceed_load_factor() const {
        if (buckets.empty()) return false;
        return static_cast<float>(num_elements + 1) / buckets.size() > max_load_factor;
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