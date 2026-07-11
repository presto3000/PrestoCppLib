#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <presto/lf_queue_mpmc.hpp>
// ------------------------------------------------------------------
// Small helper: lf::Queue has no blocking pop built in (a lock-free
// queue intentionally keeps blocking policy out of its hands -- that's
// a caller decision). This just spins with a short sleep until an
// item shows up. Good enough for demo/producer-consumer code; a real
// high-throughput consumer would usually just call try_pop() in its
// own event loop instead of blocking at all.
// ------------------------------------------------------------------
template<typename T>
T blocking_pop(lf::Queue<T>& q)
{
    T value{};
    while (!q.try_pop(value))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return value;
}

// Same idea, but gives up after a timeout instead of waiting forever.
template<typename T>
bool try_pop_for(lf::Queue<T>& q, T& out, std::chrono::milliseconds timeout)
{
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (q.try_pop(out))
        {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}

class Example
{
public:
	// File Hash Example
	void RunFileHash();
	// Thread Pool Example
	void RunThreadPool1();

	// Duplicates example
	void RunDuplicatesExample();

	// ------------------------- RingBuffer -------------------------- //
	void example_RingBuffer_basic();
	void example_RingBuffer_overwrite();
	void example_RingBuffer_nonblocking_peek();
	void example_RingBuffer_move_semantics();
	void example_RingBuffer_timeout();
	void example_RingBuffer_producer_consumer();
	void example_RingBuffer_multiple_producers();
	void example_RingBuffer_logging_buffer();
	void example_RingBuffer_try_operations();
	void example_RingBuffer_initializer_list();
	void example_RingBuffer_all();

	// ------------------------- lock free Queue -------------------------- //
	void example_LfQueue_basic();
	void example_LfQueue_full_behavior();
	void example_LfQueue_try_operations();
	void example_LfQueue_move_semantics();
	void example_LfQueue_timeout();
	void example_LfQueue_producer_consumer();
	void example_LfQueue_multiple_producers();
	void example_LfQueue_bounded_buffer();
	void example_LfQueue_statistics();
	void example_LfQueue_all();
};


