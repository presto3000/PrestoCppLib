#pragma once

#include <cstring>
#include <stdexcept>
#include <iostream>

// A simple string class that manages its own memory, similar to std::string but with a more limited feature set.
// Version 0.1: basic construction, copy/move semantics, element access, and appending.

/*
TODO:
- a bunch of stuff
*/

class PrestoString
{
private:

	char* data_;
	size_t size_;
	size_t capacity_;

	void allocateAndCopy(const char* inData, size_t len)
	{
		capacity_ = len + 1;
		data_ = new char[capacity_];
		std::memcpy(data_, inData, len);
		data_[len] = '\0'; // 'h' 'e' 'l' 'l' 'o' '\0', string becomes a valid C-string
		size_ = len;
	}

public:

	// ===== Constructors =====
	/* Empty String allocate 1 byte to be a valid C-string, it has '\0'*/
	PrestoString() : data_(nullptr), size_(0), capacity_(0) {
		allocateAndCopy("", 0);
	}
	/* This allows
	PrestoString s("hello");
	PrestoString s(nullptr);
	*/
	PrestoString(const char* str) {
		if (!str) {
			allocateAndCopy("", 0);
		}
		else
		{
			allocateAndCopy(str, std::strlen(str));
		}
	}
	PrestoString(const PrestoString& other) {
		allocateAndCopy(other.data_, other.size_);
	}

	PrestoString(PrestoString&& other) noexcept
		: data_(other.data_), size_(other.size_), capacity_(other.capacity_)
	{
		other.data_ = nullptr;
		other.size_ = 0;
		other.capacity_ = 0;
	}

	PrestoString& operator=(const PrestoString& other) {
		if (this != &other)
		{
			delete[] data_;
			allocateAndCopy(other.data_, other.size_);
			return *this;
		}
		else
		{
			return *this;
		}
	}

	PrestoString& operator=(PrestoString&& other) noexcept
	{
		if (this != &other)
		{
			delete[] data_;
			data_ = other.data_;
			size_ = other.size_;
			capacity_ = other.capacity_;
			other.data_ = nullptr;
			other.size_ = 0;
			other.capacity_ = 0;
		}
		return *this;
	}

	~PrestoString() {
		delete[] data_;
	}

	// ===== Accessors =====
	size_t size() const { return size_; }
	size_t length() const { return size_; }
	const char* c_str() const { return data_; }
	size_t getCapacity() const { return capacity_; }

	// ===== Element access =====
	char& operator[](size_t index) {
		return data_[index]; // no bounds check (like std::string)
	}

	const char& operator[](size_t index) const {
		return data_[index]; // no bounds check
	}

	char at(size_t index) const {
		if (index >= size_) throw std::out_of_range("PrestoString::at");
		return data_[index];
	}

	// ===== Append =====
	void append(const char* str) {
		size_t add_len = std::strlen(str);
		size_t new_size = size_ + add_len;

		// 1. grow capacity if needed
		if (new_size + 1 > capacity_) {

			size_t new_capacity = (capacity_ == 0) ? 1 : capacity_;

			while (new_capacity < new_size + 1) {
				new_capacity *= 2;
			}

			char* new_data = new char[new_capacity];

			// copy old data
			std::memcpy(new_data, data_, size_);

			delete[] data_;
			data_ = new_data;
			capacity_ = new_capacity;
		}

		// 2. append into existing buffer
		std::memcpy(data_ + size_, str, add_len);
		size_ = new_size;
		data_[size_] = '\0';
	}

	void append(const PrestoString& other) {
		append(other.data_);
	}

	PrestoString operator+(const PrestoString& other) const {
		PrestoString result;              // default ctor allocates
		result.size_ = size_ + other.size_;
		result.capacity_ = result.size_ + 1;
		result.data_ = new char[result.capacity_];
		std::memcpy(result.data_, data_, size_);
		std::memcpy(result.data_ + size_, other.data_, other.size_);
		result.data_[result.size_] = '\0';
		return result;
	}

	bool operator==(const PrestoString& other) const {
		if (size_ != other.size_) return false;
		return std::memcmp(data_, other.data_, size_) == 0;
	}
};

inline std::ostream& operator<<(std::ostream& os, const PrestoString& str)
{
	os << str.c_str();
	return os;
}

struct PrestoStringHash {
	size_t operator()(const PrestoString& s) const {
		size_t hash = 0;
		const char* str = s.c_str();

		while (*str) {
			hash = hash * 31 + static_cast<unsigned char>(*str);
			++str;
		}

		return hash;
	}
};