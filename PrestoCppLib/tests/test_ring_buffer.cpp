
#include "mini_test.h"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <presto/ring_buffer.hpp>

#define MINI_TEST_SUITE_NAME "ring_buffer"

// ============================================================================
// Construction
// ============================================================================

TEST(default_construction_is_empty)
{
    RingBuffer<int, 5> rb;
    CHECK(rb.empty());
    CHECK(!rb.full());
    CHECK_EQ(rb.size(), 0u);
    CHECK_EQ(rb.capacity(), 5u);
}

TEST(initializer_list_construction_populates_in_order)
{
    RingBuffer<int, 5> rb{ 1, 2, 3 };
    CHECK_EQ(rb.size(), 3u);
    CHECK_EQ(rb.get_by_value(0), 1);
    CHECK_EQ(rb.get_by_value(1), 2);
    CHECK_EQ(rb.get_by_value(2), 3);
}

TEST(initializer_list_larger_than_capacity_overwrites_oldest)
{
    RingBuffer<int, 3> rb{ 1, 2, 3, 4, 5 };
    CHECK(rb.full());
    CHECK_EQ(rb.size(), 3u);
    CHECK_EQ(rb.get_by_value(0), 3);
    CHECK_EQ(rb.get_by_value(1), 4);
    CHECK_EQ(rb.get_by_value(2), 5);
}

TEST(capacity_is_a_compile_time_constant)
{
    using RB7 = RingBuffer<int, 7>;
    static_assert(RB7::capacity() == 7, "capacity mismatch");
    CHECK_EQ(RB7::capacity(), 7u);
}

// ============================================================================
// push() -- copy / move / overwrite semantics
// ============================================================================

TEST(push_copy_grows_size_until_full)
{
    RingBuffer<int, 3> rb;
    rb.push(10);
    CHECK_EQ(rb.size(), 1u);
    rb.push(20);
    CHECK_EQ(rb.size(), 2u);
    rb.push(30);
    CHECK_EQ(rb.size(), 3u);
    CHECK(rb.full());
}

TEST(push_when_full_overwrites_oldest_element)
{
    RingBuffer<int, 3> rb{ 1, 2, 3 };
    rb.push(4);
    CHECK(rb.full());
    CHECK_EQ(rb.size(), 3u);
    CHECK_EQ(rb.get_by_value(0), 2);
    CHECK_EQ(rb.get_by_value(1), 3);
    CHECK_EQ(rb.get_by_value(2), 4);
}

TEST(push_move_stores_the_value)
{
    RingBuffer<std::string, 2> rb;
    std::string s = "hello";
    rb.push(std::move(s));
    CHECK_EQ(rb.size(), 1u);

    std::string out;
    CHECK(rb.try_front(out));
    CHECK_EQ(out, std::string("hello"));
}

// ============================================================================
// try_pop()
// ============================================================================

TEST(try_pop_fails_on_empty_buffer)
{
    RingBuffer<int, 4> rb;
    int val = 0;
    CHECK(!rb.try_pop(val));
}

TEST(try_pop_returns_elements_in_fifo_order)
{
    RingBuffer<int, 4> rb{ 1, 2, 3 };
    int val = 0;

    CHECK(rb.try_pop(val));
    CHECK_EQ(val, 1);
    CHECK(rb.try_pop(val));
    CHECK_EQ(val, 2);
    CHECK(rb.try_pop(val));
    CHECK_EQ(val, 3);
    CHECK(!rb.try_pop(val));
    CHECK(rb.empty());
}

// ============================================================================
// blocking_pop() / blocking_pop_for()
// ============================================================================

TEST(blocking_pop_returns_immediately_when_not_empty)
{
    RingBuffer<int, 4> rb{ 42 };
    int val = rb.blocking_pop();
    CHECK_EQ(val, 42);
}

TEST(blocking_pop_waits_until_another_thread_pushes)
{
    RingBuffer<int, 4> rb;

    std::thread producer([&rb]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            rb.push(99);
        });

    int val = rb.blocking_pop();
    producer.join();

    CHECK_EQ(val, 99);
}

TEST(blocking_pop_for_throws_when_it_times_out_on_empty_buffer)
{
    RingBuffer<int, 4> rb;
    bool threw = false;

    try
    {
        rb.blocking_pop_for(std::chrono::milliseconds(30));
    }
    catch (const std::logic_error&)
    {
        threw = true;
    }

    CHECK(threw);
}

TEST(blocking_pop_for_succeeds_when_value_arrives_before_timeout)
{
    RingBuffer<int, 4> rb;

    std::thread producer([&rb]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            rb.push(7);
        });

    bool threw = false;
    int val = -1;
    try
    {
        val = rb.blocking_pop_for(std::chrono::milliseconds(300));
    }
    catch (const std::logic_error&)
    {
        threw = true;
    }
    producer.join();

    CHECK(!threw);
    CHECK_EQ(val, 7);
}

// ============================================================================
// front()/back() -- blocking and non-blocking peeks
// ============================================================================

TEST(try_front_and_try_back_fail_on_empty_buffer)
{
    RingBuffer<int, 4> rb;
    int out = 0;
    CHECK(!rb.try_front(out));
    CHECK(!rb.try_back(out));
}

TEST(try_front_and_try_back_peek_without_removing)
{
    RingBuffer<int, 4> rb{ 1, 2, 3 };
    int front = -1, back = -1;

    CHECK(rb.try_front(front));
    CHECK(rb.try_back(back));
    CHECK_EQ(front, 1);
    CHECK_EQ(back, 3);
    // Peeking must not consume elements.
    CHECK_EQ(rb.size(), 3u);
}

TEST(blocking_front_and_back_return_oldest_and_newest)
{
    RingBuffer<int, 4> rb{ 5, 6, 7 };
    CHECK_EQ(rb.blocking_front(), 5);
    CHECK_EQ(rb.blocking_back(), 7);
    // Still non-destructive.
    CHECK_EQ(rb.size(), 3u);
}

// ============================================================================
// Random access: get_by_value() / get_or()
// ============================================================================

TEST(get_by_value_indexes_from_oldest_to_newest)
{
    RingBuffer<int, 5> rb{ 10, 20, 30, 40 };
    CHECK_EQ(rb.get_by_value(0), 10);
    CHECK_EQ(rb.get_by_value(1), 20);
    CHECK_EQ(rb.get_by_value(2), 30);
    CHECK_EQ(rb.get_by_value(3), 40);
}

TEST(get_by_value_throws_out_of_range_for_bad_index)
{
    RingBuffer<int, 5> rb{ 1, 2 };
    bool threw = false;

    try { rb.get_by_value(2); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    threw = false;
    try { rb.get_by_value(100); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
}

TEST(get_or_returns_default_value_when_out_of_range)
{
    RingBuffer<int, 5> rb{ 1, 2 };
    CHECK_EQ(rb.get_or(0, -1), 1);
    CHECK_EQ(rb.get_or(1, -1), 2);
    CHECK_EQ(rb.get_or(2, -1), -1);
    CHECK_EQ(rb.get_or(1000, -1), -1);
}

// ============================================================================
// clear() / empty() / full()
// ============================================================================

TEST(clear_empties_the_buffer)
{
    RingBuffer<int, 4> rb{ 1, 2, 3 };
    rb.clear();

    CHECK(rb.empty());
    CHECK_EQ(rb.size(), 0u);

    int val = 0;
    CHECK(!rb.try_pop(val));
}

TEST(clear_then_push_reuses_indices_correctly)
{
    RingBuffer<int, 3> rb{ 1, 2, 3 };
    rb.clear();
    rb.push(9);

    CHECK_EQ(rb.size(), 1u);
    CHECK_EQ(rb.get_by_value(0), 9);
}

TEST(empty_and_full_reflect_state_transitions)
{
    RingBuffer<int, 2> rb;
    CHECK(rb.empty());
    CHECK(!rb.full());

    rb.push(1);
    CHECK(!rb.empty());
    CHECK(!rb.full());

    rb.push(2);
    CHECK(!rb.empty());
    CHECK(rb.full());
}

// ============================================================================
// available() / utilization()
// ============================================================================

TEST(available_and_utilization_track_fill_level)
{
    RingBuffer<int, 4> rb;
    CHECK_EQ(rb.available(), 4u);
    CHECK_EQ(rb.utilization(), 0.0);

    rb.push(1);
    rb.push(2);
    CHECK_EQ(rb.available(), 2u);
    CHECK_EQ(rb.utilization(), 50.0);

    rb.push(3);
    rb.push(4);
    CHECK_EQ(rb.available(), 0u);
    CHECK_EQ(rb.utilization(), 100.0);
}

// ============================================================================
// Concurrency: multiple producers / consumers
// ============================================================================

TEST(concurrent_push_and_try_pop_preserve_every_element)
{
    // Capacity comfortably exceeds the total number of items pushed, so no
    // overwrite can occur -- this test is about thread-safety, not the
    // overwrite-on-full behavior (which is covered above single-threaded).
    constexpr std::size_t producers = 4;
    constexpr std::size_t items_per_producer = 2000;
    constexpr std::size_t capacity = 16384;
    constexpr std::size_t total_items = producers * items_per_producer;

    RingBuffer<long long, capacity> rb;

    std::atomic<std::size_t> popped_count{ 0 };
    std::atomic<long long> sum_popped{ 0 };

    std::vector<std::thread> prod_threads;
    prod_threads.reserve(producers);
    for (std::size_t p = 0; p < producers; ++p)
    {
        prod_threads.emplace_back([&rb, p]()
            {
                for (std::size_t i = 0; i < items_per_producer; ++i)
                    rb.push(static_cast<long long>(p * 1000000ull + i));
            });
    }

    constexpr std::size_t consumers = 2;
    std::vector<std::thread> cons_threads;
    cons_threads.reserve(consumers);
    for (std::size_t c = 0; c < consumers; ++c)
    {
        cons_threads.emplace_back([&]()
            {
                long long val = 0;
                while (popped_count.load(std::memory_order_relaxed) < total_items)
                {
                    if (rb.try_pop(val))
                    {
                        sum_popped.fetch_add(val, std::memory_order_relaxed);
                        popped_count.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            });
    }

    for (auto& t : prod_threads) t.join();
    for (auto& t : cons_threads) t.join();

    long long expected_sum = 0;
    for (std::size_t p = 0; p < producers; ++p)
        for (std::size_t i = 0; i < items_per_producer; ++i)
            expected_sum += static_cast<long long>(p * 1000000ull + i);

    CHECK_EQ(popped_count.load(), total_items);
    CHECK_EQ(sum_popped.load(), expected_sum);
    CHECK(rb.empty());
}

TEST(concurrent_push_overwrite_never_crashes_or_corrupts_size)
{
    // Small capacity, many producers hammering push() so overwrites are
    // guaranteed to happen concurrently. There's no way to predict which
    // elements survive, so this test only asserts the invariants that must
    // always hold: size never exceeds capacity, and the buffer is left in a
    // internally consistent (readable) state afterwards.
    constexpr std::size_t capacity = 8;
    RingBuffer<int, capacity> rb;

    constexpr std::size_t producers = 4;
    constexpr std::size_t items_per_thread = 5000;

    std::vector<std::thread> threads;
    threads.reserve(producers);
    for (std::size_t p = 0; p < producers; ++p)
    {
        threads.emplace_back([&rb, p]()
            {
                for (std::size_t i = 0; i < items_per_thread; ++i)
                    rb.push(static_cast<int>(p * 1000000 + i));
            });
    }
    for (auto& t : threads) t.join();

    CHECK(rb.full());
    CHECK_EQ(rb.size(), capacity);

    // Every remaining slot must be readable without throwing.
    for (std::size_t i = 0; i < rb.size(); ++i)
    {
        bool threw = false;
        try { (void)rb.get_by_value(i); }
        catch (const std::out_of_range&) { threw = true; }
        CHECK(!threw);
    }
}
