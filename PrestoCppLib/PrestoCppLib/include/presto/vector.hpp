#pragma once
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <memory>

struct Bomb {
    int value;

    Bomb(int v = 0) : value(v) {}

    Bomb(const Bomb&) {
        //throw std::runtime_error("Boom in copy!");
    }

    Bomb(Bomb&&) noexcept {
        //throw std::runtime_error("Boom in move!");
    }

    Bomb& operator=(const Bomb&) = delete;
    Bomb& operator=(Bomb&&) = delete;
};

template<class T>
class PrestoVector
{
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    static constexpr size_t INITIAL_CAP = 8; // avoid reallocs on small pushes

    void destroy_range(T* ptr, size_t count) noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < count; ++i)
                ptr[i].~T();
        }
    }

    static T* alloc(size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    static void dealloc(T* ptr) noexcept {
        ::operator delete(ptr);
    }

    void reallocate(size_t new_cap) {
        T* new_data = alloc(new_cap);

        if constexpr (std::is_trivially_copyable_v<T>) {
            // single memcpy — no loop, no construction overhead
            std::memcpy(new_data, data_, size_ * sizeof(T));
        }
        // construct new object in new memory using MOVE
        else if constexpr (std::is_nothrow_move_constructible_v<T>) {
            for (size_t i = 0; i < size_; ++i) {
                ::new(new_data + i) T(std::move(data_[i]));
                data_[i].~T();
            }
        }
        else {
            // T move can throw — copy instead and roll back on failure
            size_t i = 0;
            try {
                for (; i < size_; ++i)
                    ::new(new_data + i) T(data_[i]);
            }
            catch (...) {
                destroy_range(new_data, i);
                dealloc(new_data);
                throw;
            }
            destroy_range(data_, size_);
        }

        dealloc(data_);
        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    // Constructors
    PrestoVector() noexcept
        : data_(nullptr), size_(0), capacity_(0) {
    }

    explicit PrestoVector(size_t count)
        : data_(alloc(count)), size_(count), capacity_(count) {
        if constexpr (std::is_trivially_default_constructible_v<T>) {
            std::memset(data_, 0, count * sizeof(T));
        }
        else {
            size_t i = 0;
            try {
                for (; i < count; ++i) ::new(data_ + i) T();
            }
            catch (...) {
                destroy_range(data_, i);
                dealloc(data_);
                throw;
            }
        }
    }

    PrestoVector(std::initializer_list<T> init)
        : data_(alloc(init.size())), size_(0), capacity_(init.size()) {
        for (const auto& el : init)
            ::new(data_ + size_++) T(el);
    }

    ~PrestoVector() noexcept {
        destroy_range(data_, size_); // destroy live objects
        dealloc(data_);              // free memory once — no double-destroy
    }

    PrestoVector(const PrestoVector& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
        : data_(nullptr), size_(0), capacity_(0)
    {
        const size_t n = other.size_;
        if (n == 0) [[unlikely]] return;

        T* const raw = alloc(n);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(raw, other.data_, n * sizeof(T));
        }
        else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            for (size_t i = 0; i < n; ++i)
                ::new(raw + i) T(other.data_[i]);
        }
        else {
            size_t i = 0;
            try {
                for (; i < n; ++i)
                    ::new(raw + i) T(other.data_[i]);
            }
            catch (...) {
                destroy_range(raw, i);
                dealloc(raw);
                throw;
            }
        }

        data_ = raw;
        size_ = n;
        capacity_ = n;
    }

    PrestoVector(PrestoVector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    PrestoVector& operator=(const PrestoVector& other) {
        if (this == &other) return *this;
        PrestoVector temp(other);
        swap(temp);
        return *this;
    }

    PrestoVector& operator=(PrestoVector&& other) noexcept {
        if (this == &other) return *this;
        swap(other);
        return *this;
    }

    void swap(PrestoVector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Capacity
    size_t size()     const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool   empty()    const noexcept { return size_ == 0; }

    void reserve(size_t new_cap) {
        if (new_cap > capacity_)
            reallocate(new_cap);
    }

    void clear() noexcept {
        destroy_range(data_, size_);
        size_ = 0;
        // keep allocation alive - matches std::vector behavior
    }

    [[nodiscard]] T* data() noexcept { return data_; }
    [[nodiscard]] const T* data() const noexcept { return data_; }

    // push_back
    void push_back(const T& value) {
        if (size_ == capacity_) [[unlikely]]
            reallocate(capacity_ == 0 ? INITIAL_CAP : capacity_ * 2);
        ::new(data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) [[unlikely]]
            reallocate(capacity_ == 0 ? INITIAL_CAP : capacity_ * 2);
        ::new(data_ + size_) T(std::move(value));
        ++size_;
    }

    template<class... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) [[unlikely]]
            reallocate(capacity_ == 0 ? INITIAL_CAP : capacity_ * 2);
        T* p = ::new(data_ + size_) T(std::forward<Args>(args)...);
        ++size_;
        return *p;
    }

    void pop_back() noexcept {
        if (size_ == 0) [[unlikely]] return;
        --size_;
        if constexpr (!std::is_trivially_destructible_v<T>)
            data_[size_].~T();
    }

    // Element access
    T& operator[](size_t i) noexcept { return data_[i]; }
    const T& operator[](size_t i) const noexcept { return data_[i]; }

    T& at(size_t i) {
        if (i >= size_) throw std::out_of_range("PrestoVector::at");
        return data_[i];
    }
    const T& at(size_t i) const {
        if (i >= size_) throw std::out_of_range("PrestoVector::at");
        return data_[i];
    }

    // Iterators
    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
    const T* begin() const noexcept { return data_; }
    const T* end() const noexcept { return data_ + size_; }
    const T* cbegin() const noexcept { return data_; }
    const T* cend() const noexcept { return data_ + size_; }
};