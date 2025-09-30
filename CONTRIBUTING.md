# Contributing to YINI

First off, thank you for considering contributing to YINI! It's people like you that make open source such a great community.

## How Can I Contribute?

### Reporting Bugs
If you find a bug, please open an issue on our GitHub repository. Be sure to include a clear title, a description of the issue, and steps to reproduce it.

### Suggesting Enhancements
If you have an idea for a new feature or an improvement to an existing one, please open an issue to discuss it. This allows us to coordinate our efforts and prevent duplicated work.

### Pull Requests
We welcome pull requests for bug fixes, new features, and improvements.

## Development Setup

To get started with the YINI codebase, you will need:
- A C++17 compliant compiler (e.g., GCC, Clang, MSVC)
- CMake (version 3.10 or higher)
- Git

### Build Instructions

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/your-repo/yini.git
    cd yini
    ```

2.  **Configure CMake:**
    ```sh
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    ```
    *Use `Debug` for development and `Release` for benchmarking.*

3.  **Build the project:**
    ```sh
    cmake --build build
    ```

### Running Tests

All tests are built into the `yini_tests` executable and can be run via CTest.

```sh
cd build
ctest --output-on-failure
```

### Running Benchmarks

The performance benchmarks are only built in `Release` mode.

```sh
# First, configure and build in Release mode
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the benchmark executable
./build/bin/yini_benchmark
```

## Coding Conventions
Please follow the coding conventions outlined in the `YINI.md` document. This includes our naming conventions and brace style to ensure the codebase remains consistent and readable.

We look forward to your contributions!