# Building Boost.SafeProfile

## Prerequisites

### Required
- **C++20 compiler:**
  - GCC 10+ or
  - Clang 10+ or
  - MSVC 2019+ (16.11+)
- **CMake 3.20+**
- **Boost 1.82+** (with `program_options`, `filesystem`, `json`, `unit_test_framework`)

### Optional (later phases)
- **LLVM/Clang 15+** (for static analysis engine)
- **Git** (for repository ingestion)

## Build Instructions

### Linux/macOS

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Install (optional)
sudo cmake --install build --prefix /usr/local
```

### Debug Build with Sanitizers

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBOOST_SAFEPROFILE_ENABLE_SANITIZERS=ON

cmake --build build -j$(nproc)
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

Or directly:

```bash
./build/tests/unit_tests
```

## Installation via Homebrew (macOS)

If you need to install dependencies:

```bash
# Install CMake and Boost
brew install cmake boost

# Verify versions
cmake --version  # Should be 3.20+
brew list --versions boost  # Should be 1.82+
```

## Installation via apt (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install cmake libboost-all-dev

# Verify versions
cmake --version
dpkg -l | grep libboost
```

## Troubleshooting

### Boost not found
Ensure `BOOST_ROOT` is set or pass `-DBOOST_ROOT=/path/to/boost` to CMake.

### Compiler too old
Check your compiler version:
```bash
g++ --version    # GCC
clang++ --version  # Clang
```

Upgrade if necessary or specify a newer compiler:
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-11
```

### Sanitizer issues
If sanitizers cause problems, disable them:
```bash
cmake -S . -B build -DBOOST_SAFEPROFILE_ENABLE_SANITIZERS=OFF
```
