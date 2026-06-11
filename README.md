# PrestoCppLib

A modern C++ header-only library providing custom containers, smart pointers, and utilities including SHA-256 file hashing.

---

## ✨ Features

### Containers
- `PrestoVector<T>`
- `PrestoString`
- `PrestoUnorderedMap<T>`

### Smart Pointers
- `PrestoUniquePtr<T>`
- `PrestoSharedPtr<T>`
- `PrestoWeakPtr<T>`

### Hashing
- `sha256File(path)` — SHA-256 file hashing using OpenSSL

### Benchmarks
- Performance benchmarks included in `/benchmarks`

### Build System
- CMake-based build system
- Header-only design (no compilation required for library)

---

## Dependencies

This library uses **OpenSSL** for hashing functionality:

- Required: OpenSSL (`libssl`, `libcrypto`)

### Install via vcpkg (recommended)
```bash
vcpkg install openssl

