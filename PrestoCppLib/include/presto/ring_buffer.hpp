#pragma once

#include <stdexcept>
#include <initializer_list>
#include <array>
#include <utility>
#include <type_traits>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include <chrono>

/**
 * @brief Thread-safe, fixed-size circular buffer (ring buffer).
 *
 * A circular queue that automatically overwrites the oldest data when full.
 * All operations are protected by a mutex and use condition variables for
 * efficient blocking operations.
 *
 * @tparam ValueType The type of elements stored in the buffer
 * @tparam Capacity The maximum number of elements (enforced at compile-time)
 *
 * Thread Safety: All public methods are thread-safe. The internal mutex
 * protects all access to the buffer and indices.
 *
 * @example
 * ring_buffer<int, 10> rb;
 * rb.push(42);
 * int value = rb.blocking_pop();  // Waits if buffer is empty
 */
template <typename ValueType, std::size_t Capacity> requires (Capacity > 0)
class RingBuffer
{
public:

	RingBuffer()
		: m_readIndex(0), m_writeIndex(0), m_size(0)
	{
	}

	RingBuffer(std::initializer_list<ValueType> data)
		: RingBuffer{}
	{
		for (const ValueType& value : data)
			push(value);
	}

	// Delete copy constructor and assignment (mutex is not copyable)
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator=(const RingBuffer&) = delete;
	// Delete move constructor and assignment (mutex is not movable)
	RingBuffer(RingBuffer&&) = delete;
	RingBuffer& operator=(RingBuffer&&) = delete;

	~RingBuffer() = default;

	// =========================================================================
	// PUSH OPERATIONS
	// =========================================================================

	/**
	 * @brief Pushes a value into the buffer (copy).
	 *
	 * If the buffer is full, the oldest element is silently discarded.
	 * This is the defining characteristic of a ring buffer.
	 *
	 * Thread-safe: Protected by mutex.
	 * Exception-safe: No-throw guarantee.
	 * Notifies any threads waiting on blocking_pop().
	 *
	 * @param value The value to push (copied)
	 */
	void push(const ValueType& value)
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		// Write to current position
		m_data[m_writeIndex] = value;
		m_writeIndex = (m_writeIndex + 1) % Capacity;

		// If buffer was full, advance read index (discard oldest)
		if (m_size == Capacity)
			m_readIndex = (m_readIndex + 1) % Capacity;
		else
			++m_size;  // Buffer not full yet, just increment size

		// Wake up any thread waiting on blocking_pop()
		m_nonEmptyCv.notify_one();
	}

	/**
	 * @brief Pushes a value into the buffer (move).
	 *
	 * More efficient than copy push for move-constructible types.
	 * If the buffer is full, the oldest element is discarded.
	 *
	 * Thread-safe: Protected by mutex.
	 * Exception-safe: No-throw guarantee (if move is no-throw).
	 * Notifies waiting threads.
	 *
	 * @param value The value to push (moved)
	 */
	void push(ValueType&& value)
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		m_data[m_writeIndex] = std::move(value);
		m_writeIndex = (m_writeIndex + 1) % Capacity;

		if (m_size == Capacity)
			m_readIndex = (m_readIndex + 1) % Capacity;
		else
			++m_size;

		m_nonEmptyCv.notify_one();
	}

	// =========================================================================
	// POP OPERATIONS - BLOCKING
	// =========================================================================

	/**
	 * @brief Removes and returns the oldest element (blocking).
	 *
	 * Waits indefinitely if the buffer is empty. Returns as soon as
	 * an element becomes available.
	 *
	 * Thread-safe: Protected by mutex and condition variable.
	 * Blocking: Will wait if buffer is empty.
	 *
	 * @return The oldest element in the buffer
	 *
	 * @note Returns by value to avoid dangling references after unlock.
	 * @note Spurious wakeups are handled automatically.
	 */
	ValueType blocking_pop()
	{
		std::unique_lock<std::mutex> lck{ m_mtx };

		// Wait for buffer to have at least one element
		// Lambda will be re-checked if spurious wakeup occurs
		m_nonEmptyCv.wait(lck, [this]() { return m_size > 0; });

		const ValueType output{ m_data[m_readIndex] };
		m_readIndex = (m_readIndex + 1) % Capacity;
		--m_size;

		return output;
	}

	/**
	 * @brief Removes and returns the oldest element with timeout (blocking).
	 *
	 * Waits up to the specified duration for an element to be available.
	 *
	 * Thread-safe: Protected by mutex and condition variable.
	 *
	 * @tparam Rep The representation of the duration
	 * @tparam Period The period of the duration
	 * @param timeout The maximum time to wait
	 * @return The oldest element, or a default-constructed value if timeout
	 * @throw std::logic_error if buffer is still empty after timeout
	 */
	template<typename Rep, typename Period>
	ValueType blocking_pop_for(const std::chrono::duration<Rep, Period>& timeout)
	{
		std::unique_lock<std::mutex> lck{ m_mtx };

		if (!m_nonEmptyCv.wait_for(lck, timeout, [this]() { return m_size > 0; })) {
			throw std::logic_error{ "Timeout: buffer still empty" };
		}

		const ValueType output{ m_data[m_readIndex] };
		m_readIndex = (m_readIndex + 1) % Capacity;
		--m_size;

		return output;
	}

	/**
	 * @brief Attempts to pop an element without blocking.
	 *
	 * Returns immediately with true if an element was popped,
	 * or false if the buffer is empty.
	 *
	 * Thread-safe: Protected by mutex.
	 * Non-blocking: Returns immediately.
	 *
	 * @param out Reference to store the popped value
	 * @return true if an element was successfully popped, false if empty
	 */
	bool try_pop(ValueType& out)
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		if (m_size == 0)
			return false;

		out = m_data[m_readIndex];
		m_readIndex = (m_readIndex + 1) % Capacity;
		--m_size;

		return true;
	}

	// =========================================================================
	// FRONT/BACK ACCESS - BLOCKING
	// =========================================================================

	/**
	 * @brief Returns a copy of the oldest element (blocking).
	 *
	 * Waits indefinitely if the buffer is empty. Peeks at the front
	 * without removing it.
	 *
	 * Thread-safe: Protected by mutex and condition variable.
	 * Blocking: Waits if buffer is empty.
	 *
	 * @return A copy of the oldest element
	 */
	ValueType blocking_front() const
	{
		std::unique_lock<std::mutex> lck{ m_mtx };
		m_nonEmptyCv.wait(lck, [this]() { return m_size > 0; });
		return m_data[m_readIndex];
	}

	/**
	 * @brief Returns a copy of the newest element (blocking).
	 *
	 * Waits indefinitely if the buffer is empty. Peeks at the back
	 * without removing it.
	 *
	 * Thread-safe: Protected by mutex and condition variable.
	 * Blocking: Waits if buffer is empty.
	 *
	 * @return A copy of the newest element
	 */
	ValueType blocking_back() const
	{
		std::unique_lock<std::mutex> lck{ m_mtx };
		m_nonEmptyCv.wait(lck, [this]() { return m_size > 0; });
		return m_data[(m_readIndex + (m_size - 1)) % Capacity];
	}

	/**
	 * @brief Non-blocking peek at the front element.
	 *
	 * Returns immediately. Returns the oldest element without removing it.
	 *
	 * Thread-safe: Protected by mutex.
	 * Non-blocking: Returns immediately.
	 *
	 * @param out Reference to store the front value
	 * @return true if buffer is not empty and value was retrieved, false otherwise
	 */
	bool try_front(ValueType& out) const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		if (m_size == 0)
			return false;

		out = m_data[m_readIndex];
		return true;
	}

	/**
	 * @brief Non-blocking peek at the back element.
	 *
	 * Returns immediately. Returns the newest element without removing it.
	 *
	 * Thread-safe: Protected by mutex.
	 * Non-blocking: Returns immediately.
	 *
	 * @param out Reference to store the back value
	 * @return true if buffer is not empty and value was retrieved, false otherwise
	 */
	bool try_back(ValueType& out) const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		if (m_size == 0)
			return false;

		out = m_data[(m_readIndex + (m_size - 1)) % Capacity];
		return true;
	}

	// =========================================================================
	// RANDOM ACCESS
	// =========================================================================

	/**
	 * @brief Accesses element at logical index by value.
	 *
	 * Index 0 is the front (oldest), index size()-1 is the back (newest).
	 * The internal circular position is handled transparently.
	 *
	 * Thread-safe: Protected by mutex.
	 *
	 * @param index The logical index (0-based from front)
	 * @return A copy of the element at index
	 * @throw std::out_of_range if index >= size()
	 */
	ValueType get_by_value(std::size_t index) const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		if (index >= m_size)
			throw std::out_of_range{ "Index out of bounds" };

		return m_data[(m_readIndex + index) % Capacity];
	}

	/**
	 * @brief Safely accesses element at logical index with fallback.
	 *
	 * Returns the element at index, or a default-constructed value if out of bounds.
	 *
	 * Thread-safe: Protected by mutex.
	 *
	 * @param index The logical index (0-based from front)
	 * @param default_value The value to return if index is out of bounds
	 * @return Element at index, or default_value if out of bounds
	 */
	ValueType get_or(std::size_t index, const ValueType& default_value) const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };

		if (index >= m_size)
			return default_value;

		return m_data[(m_readIndex + index) % Capacity];
	}

	// =========================================================================
	// UTILITY OPERATIONS
	// =========================================================================

	void clear()
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		m_readIndex = m_writeIndex = m_size = 0;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		return m_size == 0;
	}

	bool full() const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		return m_size == Capacity;
	}

	std::size_t size() const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		return m_size;
	}

	/**
	 * @brief Returns the maximum capacity of the buffer.
	 *
	 * Thread-safe: No lock needed (compile-time constant).
	 *
	 * @return The capacity (template parameter)
	 */
	static constexpr std::size_t capacity() noexcept
	{
		return Capacity;
	}

	/**
	 * @brief Returns the number of available slots.
	 *
	 * Thread-safe: Protected by mutex.
	 *
	 * @return capacity() - size()
	 */
	std::size_t available() const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		return Capacity - m_size;
	}

	/**
	 * @brief Returns the utilization percentage.
	 *
	 * Thread-safe: Protected by mutex.
	 *
	 * @return Percentage from 0.0 to 100.0
	 */
	double utilization() const
	{
		std::lock_guard<std::mutex> lck{ m_mtx };
		return (m_size * 100.0) / Capacity;
	}

private:

	mutable std::mutex m_mtx;                    ///< Protects all access
	mutable std::condition_variable m_nonEmptyCv;  ///< Signals when buffer is non-empty
	std::array<ValueType, Capacity> m_data;      ///< Circular buffer storage
	std::size_t m_readIndex;                     ///< Index of oldest element
	std::size_t m_writeIndex;                    ///< Index where next write goes
	std::size_t m_size;                          ///< Current number of elements
};
