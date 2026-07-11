#include "examples.h"
#include <string>
#include <presto/hashing/file_hash.hpp>
#include <presto/concurrency/thread_pool.hpp>
#include <presto/vector.hpp>
#include <presto/algorithms/algorithms.hpp>
#include <presto/string.hpp>
#include <presto/ring_buffer.hpp>


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

// ============================================================================
// EXAMPLE 1: Basic Single-Threaded Usage
// ============================================================================
void Example::example_RingBuffer_basic()
{
	std::cout << "\n=== EXAMPLE 1: Basic Usage ===\n";

	RingBuffer<int, 5> rb;

	// Push some values
	for (int i = 10; i <= 50; i += 10) {
		rb.push(i);
		std::cout << "Pushed " << i << ", size: " << rb.size() << "\n";
	}

	// Non-blocking pop
	int val;
	if (rb.try_pop(val)) {
		std::cout << "Popped: " << val << "\n";
	}

	// Check capacity
	std::cout << "Current utilization: " << rb.utilization() << "%\n";
	std::cout << "Available slots: " << rb.available() << "\n";
}

// ============================================================================
// EXAMPLE 2: Ring Buffer Overwrite Behavior
// ============================================================================
void Example::example_RingBuffer_overwrite()
{
	std::cout << "\n=== EXAMPLE 2: Overwrite When Full ===\n";

	RingBuffer<int, 3> rb;

	std::cout << "Buffer capacity: " << rb.capacity() << "\n";

	// Fill and overfill
	for (int i = 1; i <= 5; ++i) {
		rb.push(i * 100);
		std::cout << "Pushed " << (i * 100) << ", size: " << rb.size() << "\n";

		// Show current contents
		std::cout << "  Contents: ";
		for (size_t j = 0; j < rb.size(); ++j) {
			std::cout << rb.get_by_value(j) << " ";
		}
		std::cout << "\n";
	}

	std::cout << "Notice: oldest values were automatically discarded\n";
}

// ============================================================================
// EXAMPLE 3: Non-Blocking Peek Operations
// ============================================================================
void Example::example_RingBuffer_nonblocking_peek()
{
	std::cout << "\n=== EXAMPLE 3: Non-Blocking Peek ===\n";

	RingBuffer<std::string, 4> rb{ "first", "second", "third" };

	// Try to peek at front
	std::string front;
	if (rb.try_front(front)) {
		std::cout << "Front: " << front << "\n";
	}

	// Try to peek at back
	std::string back;
	if (rb.try_back(back)) {
		std::cout << "Back: " << back << "\n";
	}

	// Access by index
	std::cout << "Element at index 1: " << rb.get_by_value(1) << "\n";

	// Safe access with default value
	std::cout << "Element at index 10 (out of bounds): "
		<< rb.get_or(10, std::string("DEFAULT")) << "\n";
}

// ============================================================================
// EXAMPLE 4: Move Semantics
// ============================================================================
void Example::example_RingBuffer_move_semantics()
{
	std::cout << "\n=== EXAMPLE 4: Move Semantics ===\n";

	RingBuffer<std::string, 5> rb;

	// Copy push
	std::string s1 = "copy";
	rb.push(s1);
	std::cout << "After copy push, s1 = \"" << s1 << "\"\n";

	// Move push
	std::string s2 = "move";
	rb.push(std::move(s2));
	std::cout << "After move push, s2 = \"" << s2 << "\"\n";  // s2 is now empty

	std::cout << "Buffer size: " << rb.size() << "\n";
}

// ============================================================================
// EXAMPLE 5: Timeout Operations
// ============================================================================
void Example::example_RingBuffer_timeout()
{
	std::cout << "\n=== EXAMPLE 5: Timeout Pop ===\n";

	RingBuffer<int, 5> rb;
	rb.push(42);

	try {
		// Pop with a timeout (should succeed immediately)
		int val = rb.blocking_pop_for(std::chrono::milliseconds(100));
		std::cout << "Successfully popped: " << val << "\n";
	}
	catch (const std::logic_error& e) {
		std::cout << "Error: " << e.what() << "\n";
	}

	try {
		// Try to pop from empty buffer with short timeout (should fail)
		std::cout << "Waiting up to 100ms for data...\n";
		int val = rb.blocking_pop_for(std::chrono::milliseconds(100));
	}
	catch (const std::logic_error& e) {
		std::cout << "Got expected timeout: " << e.what() << "\n";
	}
}

// ============================================================================
// EXAMPLE 6: Producer-Consumer Pattern
// ============================================================================
void Example::example_RingBuffer_producer_consumer()
{
	std::cout << "\n=== EXAMPLE 6: Producer-Consumer (Multi-threaded) ===\n";

	RingBuffer<int, 10> rb;

	// Producer thread
	auto producer = [&rb]() {
		for (int i = 1; i <= 5; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			rb.push(i * 100);
			std::cout << "  [Producer] Pushed " << (i * 100) << "\n";
		}
		};

	// Consumer thread
	auto consumer = [&rb]() {
		for (int i = 0; i < 5; ++i) {
			int val = rb.blocking_pop();  // Wait for data
			std::cout << "  [Consumer] Popped " << val << "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		};

	std::thread t1(producer);
	std::thread t2(consumer);

	t1.join();
	t2.join();

	std::cout << "Producer-consumer completed\n";
}

// ============================================================================
// EXAMPLE 7: Multiple Producers
// ============================================================================
void Example::example_RingBuffer_multiple_producers()
{
	std::cout << "\n=== EXAMPLE 7: Multiple Producers ===\n";

	RingBuffer<int, 20> rb;

	auto producer = [&rb](int id) {
		for (int i = 0; i < 3; ++i) {
			int val = id * 1000 + i;
			rb.push(val);
			std::cout << "  [Producer " << id << "] Pushed " << val << "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		};

	auto consumer = [&rb]() {
		for (int i = 0; i < 9; ++i) {
			int val = rb.blocking_pop();
			std::cout << "  [Consumer] Got " << val << "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(75));
		}
		};

	std::vector<std::thread> producers;
	for (int i = 1; i <= 3; ++i) {
		producers.emplace_back(producer, i);
	}

	std::thread consumer_thread(consumer);

	for (auto& t : producers) {
		t.join();
	}
	consumer_thread.join();

	std::cout << "Multi-producer demo completed\n";
}

// ============================================================================
// EXAMPLE 8: Circular Logging Buffer
// ============================================================================
void Example::example_RingBuffer_logging_buffer()
{
	std::cout << "\n=== EXAMPLE 8: Circular Logging Buffer ===\n";

	RingBuffer<std::string, 5> logs;

	// Simulate logging
	for (int i = 1; i <= 7; ++i) {
		logs.push("Log message #" + std::to_string(i));
		std::cout << "Added log " << i << ", buffer size: " << logs.size() << "\n";
	}

	// Display all logs (oldest to newest)
	std::cout << "Recent logs:\n";
	for (size_t i = 0; i < logs.size(); ++i) {
		std::cout << "  [" << i << "] " << logs.get_by_value(i) << "\n";
	}
}

// ============================================================================
// EXAMPLE 9: Try Operations
// ============================================================================
void Example::example_RingBuffer_try_operations()
{
	std::cout << "\n=== EXAMPLE 9: Non-Blocking Try Operations ===\n";

	RingBuffer<int, 5> rb;

	// Try pop on empty buffer
	int val;
	if (rb.try_pop(val)) {
		std::cout << "Popped: " << val << "\n";
	}
	else {
		std::cout << "Buffer was empty, pop failed\n";
	}

	// Add data and try again
	rb.push(42);
	if (rb.try_pop(val)) {
		std::cout << "Successfully popped: " << val << "\n";
	}

	// Try front on empty buffer
	if (rb.try_front(val)) {
		std::cout << "Front: " << val << "\n";
	}
	else {
		std::cout << "Buffer empty, try_front failed\n";
	}
}

// ============================================================================
// EXAMPLE 10: Initializer List
// ============================================================================
void Example::example_RingBuffer_initializer_list()
{
	std::cout << "\n=== EXAMPLE 10: Initializer List Construction ===\n";

	RingBuffer<int, 10> rb{ 10, 20, 30, 40, 50 };

	std::cout << "Initialized with 5 values\n";
	std::cout << "Size: " << rb.size() << "\n";
	std::cout << "Front: " << rb.blocking_front() << "\n";
	std::cout << "Back: " << rb.blocking_back() << "\n";
}

// ============================================================================
// EXAMPLE 11: Queue Statistics
// ============================================================================
void example_statistics()
{
	std::cout << "\n=== EXAMPLE 11: Queue Statistics ===\n";

	RingBuffer<double, 100> measurements;

	// Simulate sensor readings
	for (int i = 0; i < 35; ++i) {
		measurements.push(20.5 + (i % 10) * 0.5);
	}

	std::cout << "Queue Statistics:\n";
	std::cout << "  Capacity:      " << measurements.capacity() << "\n";
	std::cout << "  Current size:  " << measurements.size() << "\n";
	std::cout << "  Available:     " << measurements.available() << "\n";
	std::cout << "  Utilization:   " << measurements.utilization() << "%\n";
	std::cout << "  Is empty?      " << (measurements.empty() ? "Yes" : "No") << "\n";
	std::cout << "  Is full?       " << (measurements.full() ? "Yes" : "No") << "\n";
}


void Example::example_RingBuffer_all()
{
	std::cout << "=============================\n";
	std::cout << " RingBuffer SUITE\n";
	std::cout << "=============================\n\n";

	example_RingBuffer_basic();
	example_RingBuffer_overwrite();
	example_RingBuffer_nonblocking_peek();
	example_RingBuffer_move_semantics();
	example_RingBuffer_timeout();
	example_RingBuffer_producer_consumer();
	example_RingBuffer_multiple_producers();
	example_RingBuffer_logging_buffer();
	example_RingBuffer_try_operations();
	example_RingBuffer_initializer_list();

	std::cout << "=============================\n";
	std::cout << " RingBuffer END SUITE\n";
	std::cout << "=============================\n\n";
}


// ========================================================================
// EXAMPLE 1: Basic Single-Threaded Usage
// ========================================================================
void Example::example_LfQueue_basic()
{
    std::cout << "\n=== EXAMPLE 1: Basic Usage ===\n";

    lf::Queue<int> q(8); // capacity must be a power of two

    for (int i = 10; i <= 50; i += 10)
    {
        bool ok = q.try_push(i);
        std::cout << "Pushed " << i << " -> " << (ok ? "ok" : "FULL")
            << ", approx size: " << q.approximate_size() << "\n";
    }

    int val;
    if (q.try_pop(val))
    {
        std::cout << "Popped: " << val << "\n";
    }

    std::cout << "Capacity: " << q.capacity() << "\n";
    std::cout << "Utilization: " << q.utilization() << "%\n";
}

// ========================================================================
// EXAMPLE 2: Behavior When Full (no overwrite -- unlike RingBuffer)
// ========================================================================
void Example::example_LfQueue_full_behavior()
{
    std::cout << "\n=== EXAMPLE 2: Behavior When Full ===\n";

    lf::Queue<int> q(4);
    std::cout << "Queue capacity: " << q.capacity() << "\n";

    for (int i = 1; i <= 6; ++i)
    {
        bool ok = q.try_push(i * 100);
        std::cout << "try_push(" << (i * 100) << ") -> "
            << (ok ? "ok" : "REJECTED (queue full)")
            << ", approx size: " << q.approximate_size() << "\n";
    }

    std::cout << "Notice: unlike a RingBuffer that overwrites the oldest\n"
        "entry, lf::Queue never silently discards data -- once\n"
        "full, try_push() just reports failure and the caller\n"
        "decides what to do (retry, drop, block, etc.).\n";
}

// ========================================================================
// EXAMPLE 3: Non-Blocking Try Operations
// ========================================================================
void Example::example_LfQueue_try_operations()
{
    std::cout << "\n=== EXAMPLE 3: Non-Blocking Try Operations ===\n";

    lf::Queue<int> q(4);

    int val;
    if (q.try_pop(val))
    {
        std::cout << "Popped: " << val << "\n";
    }
    else
    {
        std::cout << "Queue was empty, try_pop failed as expected\n";
    }

    q.try_push(42);
    if (q.try_pop(val))
    {
        std::cout << "Successfully popped: " << val << "\n";
    }

    if (!q.try_pop(val))
    {
        std::cout << "Queue empty again, try_pop correctly failed\n";
    }
}

// ========================================================================
// EXAMPLE 4: Move Semantics
// ========================================================================
void Example::example_LfQueue_move_semantics()
{
    std::cout << "\n=== EXAMPLE 4: Move Semantics ===\n";

    lf::Queue<std::string> q(8);

    // NOTE: only *move* push compiles here, not copy push. try_push()
    // requires T's constructor to be non-throwing (see the
    // static_assert in lf_queue.hpp) because if it threw after the
    // slot's CAS had already claimed it, the queue would be
    // permanently stuck. std::string's copy constructor CAN throw
    // (it allocates), so `q.try_push(some_lvalue_string)` is a
    // compile error by design -- only moves/rvalues are accepted:
    std::string s2 = "move";
    q.try_push(std::move(s2));
    std::cout << "After move push, s2 = \"" << s2 << "\" (moved-from)\n";

    q.try_push(std::string("temporary")); // a prvalue is moved too

    std::cout << "Approx size: " << q.approximate_size() << "\n";

    std::string out;
    while (q.try_pop(out))
    {
        std::cout << "  popped: \"" << out << "\"\n";
    }
}

// ========================================================================
// EXAMPLE 5: Timeout Pop (built on try_pop, since lf::Queue has no
// blocking API of its own)
// ========================================================================
void Example::example_LfQueue_timeout()
{
    std::cout << "\n=== EXAMPLE 5: Timeout Pop ===\n";

    lf::Queue<int> q(4);
    q.try_push(42);

    int val;
    if (try_pop_for(q, val, std::chrono::milliseconds(100)))
    {
        std::cout << "Successfully popped: " << val << "\n";
    }
    else
    {
        std::cout << "Error: timed out (unexpected here)\n";
    }

    std::cout << "Waiting up to 100ms for data that never arrives...\n";
    if (!try_pop_for(q, val, std::chrono::milliseconds(100)))
    {
        std::cout << "Got expected timeout\n";
    }
}

// ========================================================================
// EXAMPLE 6: Producer-Consumer Pattern
// ========================================================================
void Example::example_LfQueue_producer_consumer()
{
    std::cout << "\n=== EXAMPLE 6: Producer-Consumer (Multi-threaded) ===\n";

    lf::Queue<int> q(16);

    auto producer = [&q]() {
        for (int i = 1; i <= 5; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            while (!q.try_push(i * 100))
            {
                std::this_thread::yield(); // full: back off and retry
            }
            std::cout << "  [Producer] Pushed " << (i * 100) << "\n";
        }
        };

    auto consumer = [&q]() {
        for (int i = 0; i < 5; ++i)
        {
            int val = blocking_pop(q);
            std::cout << "  [Consumer] Popped " << val << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        };

    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();

    std::cout << "Producer-consumer completed\n";
}

// ========================================================================
// EXAMPLE 7: Multiple Producers, One Consumer
// ========================================================================
void Example::example_LfQueue_multiple_producers()
{
    std::cout << "\n=== EXAMPLE 7: Multiple Producers ===\n";

    lf::Queue<int> q(32);

    auto producer = [&q](int id) {
        for (int i = 0; i < 3; ++i)
        {
            int val = id * 1000 + i;
            while (!q.try_push(val))
            {
                std::this_thread::yield();
            }
            std::cout << "  [Producer " << id << "] Pushed " << val << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        };

    auto consumer = [&q]() {
        for (int i = 0; i < 9; ++i)
        {
            int val = blocking_pop(q);
            std::cout << "  [Consumer] Got " << val << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        };

    std::vector<std::thread> producers;
    for (int id = 1; id <= 3; ++id)
    {
        producers.emplace_back(producer, id);
    }
    std::thread consumer_thread(consumer);

    for (auto& t : producers) t.join();
    consumer_thread.join();

    std::cout << "Multi-producer demo completed\n";
}

// ========================================================================
// EXAMPLE 8: Bounded Buffer Behavior Under Load (no auto-discard)
// ========================================================================
void Example::example_LfQueue_bounded_buffer()
{
    std::cout << "\n=== EXAMPLE 8: Bounded Buffer Under Load ===\n";

    lf::Queue<std::string> logs(4); // small on purpose

    int accepted = 0, rejected = 0;
    for (int i = 1; i <= 7; ++i)
    {
        bool ok = logs.try_push("Log message #" + std::to_string(i));
        ok ? ++accepted : ++rejected;
        std::cout << "Log " << i << " -> " << (ok ? "accepted" : "REJECTED")
            << ", approx size: " << logs.approximate_size() << "\n";
    }

    std::cout << accepted << " accepted, " << rejected << " rejected.\n"
        "Notice: unlike RingBuffer's auto-discard-oldest, a\n"
        "real logger over lf::Queue would need its own policy\n"
        "for what to do when try_push() fails (e.g. drop-newest,\n"
        "block briefly, or spill to a slower path).\n";

    std::cout << "Draining remaining logs:\n";
    std::string entry;
    while (logs.try_pop(entry))
    {
        std::cout << "  " << entry << "\n";
    }
}

// ========================================================================
// EXAMPLE 9: Queue Statistics (all advisory / best-effort)
// ========================================================================
void Example::example_LfQueue_statistics()
{
    std::cout << "\n=== EXAMPLE 9: Queue Statistics (advisory) ===\n";

    lf::Queue<double> measurements(128);

    for (int i = 0; i < 35; ++i)
    {
        measurements.try_push(20.5 + (i % 10) * 0.5);
    }

    std::cout << "Queue Statistics:\n";
    std::cout << "  Capacity:            " << measurements.capacity() << "\n";
    std::cout << "  Approx size:         " << measurements.approximate_size() << "\n";
    std::cout << "  Approx utilization:  " << measurements.utilization() << "%\n";
    std::cout << "  Approx empty?        " << (measurements.approximate_empty() ? "Yes" : "No") << "\n";
    std::cout << "  Approx full?         " << (measurements.approximate_full() ? "Yes" : "No") << "\n";
    std::cout << "(\"Approx\" because other threads could push/pop between\n"
        " reading this value and acting on it -- treat it as a\n"
        " metrics snapshot, never as a precondition for correctness.)\n";
}

void Example::example_LfQueue_all()
{
    std::cout << "=============================\n";
    std::cout << " lf::Queue EXAMPLE SUITE\n";
    std::cout << "=============================\n";

    example_LfQueue_basic();
    example_LfQueue_full_behavior();
    example_LfQueue_try_operations();
    example_LfQueue_move_semantics();
    example_LfQueue_timeout();
    example_LfQueue_producer_consumer();
    example_LfQueue_multiple_producers();
    example_LfQueue_bounded_buffer();
    example_LfQueue_statistics();

    std::cout << "=============================\n";
    std::cout << " lf::Queue END SUITE\n";
    std::cout << "=============================\n\n";
}
