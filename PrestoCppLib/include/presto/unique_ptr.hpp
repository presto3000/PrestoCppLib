#include <utility>
#include <cstddef>

template <typename T>
class PrestoUniquePtr {
private:
    T* ptr;

public:
    PrestoUniquePtr() : ptr(nullptr) {}
    explicit PrestoUniquePtr(T* p) : ptr(p) {}

    ~PrestoUniquePtr() {
        delete ptr;
    }

    // no copy
    PrestoUniquePtr(const PrestoUniquePtr&) = delete;
    PrestoUniquePtr& operator=(const PrestoUniquePtr&) = delete;

    // move support
    PrestoUniquePtr(PrestoUniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    PrestoUniquePtr& operator=(PrestoUniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T* get() const { return ptr; }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    // give up ownership without deleting
    T* release() {
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    // replace managed obj
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }

    explicit operator bool() const {
        return ptr != nullptr;
    }
};
