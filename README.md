# PrestoCppLib

A modern C++ header-only library providing custom containers, smart pointers, concurrency utilities, and hashing functionality including SHA-256 file hashing.

---

## Features

### Containers
- `PrestoVector<T>`
- `PrestoString`
- `PrestoUnorderedMap<T>`

### Smart Pointers
- `PrestoUniquePtr<T>`
- `PrestoSharedPtr<T>`
- `PrestoWeakPtr<T>`

### Hashing
- `sha256File(path)` - SHA-256 file hashing using OpenSSL

### Concurrency
- `ThreadPool` - basic C++ thread pool for parallel task execution

### Benchmarks
- Performance benchmarks included in `/benchmarks`

### Build System
- CMake-based build system
- Header-only design

---

## Dependencies

This library uses **OpenSSL** for hashing functionality:

- Required: OpenSSL (`libssl`, `libcrypto`)

### Install via vcpkg (recommended)
```bash
vcpkg install openssl

