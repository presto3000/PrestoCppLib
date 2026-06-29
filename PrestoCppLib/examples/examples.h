#pragma once
#include <iostream>

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
};