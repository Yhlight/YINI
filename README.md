# YINI - An Enhanced INI Parser for Game Development

YINI is a modern, feature-rich configuration file parser implemented in C++17. It extends the standard INI format with advanced features required for game development, such as section inheritance, typed values, arithmetic operations, and macros.

This project provides a robust, cross-platform core library (`yini`) and a C-style API for easy integration with other languages, with a primary focus on C# for use in game engines like Unity.

## Features

This implementation of YINI supports the following features as described in `YINI.md`:

- **Modern C++17 Implementation:** Built with best practices, including smart pointers for memory management.
- **Extended Syntax:**
    - `//` and `/* */` style comments.
    - Typed values: Integers, Floats, Booleans, and Strings.
    - Arithmetic operations (`+`, `-`, `*`, `/`, `%`) with correct operator precedence.
    - Macro definitions (`[#define]`) and usage (`@name`).
    - `Dyna(value)` syntax for dynamic values (parser support implemented).
- **Cross-Platform C API:** A stable C-style API (`yini.h`) allows the core library to be used from C#, Python, and other languages.
- **C# P/Invoke Wrapper:** A sample C# class (`Yini.cs`) is provided to demonstrate how to use the native library.

## How to Build

The project uses [CMake](https://cmake.org/) to manage the build process, which allows it to be built on Windows, macOS, and Linux with a variety of compilers.

### Prerequisites

- A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
- CMake (version 3.15 or higher).
- A build tool like Make, Ninja, or Visual Studio.

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd <repository-directory>
    ```

2.  **Create a build directory:**
    It's best practice to perform an out-of-source build.
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake to configure the project:**
    ```bash
    cmake ..
    ```
    On Windows, you may need to specify a generator, e.g., `cmake .. -G "Visual Studio 17 2022"`.

4.  **Compile the code:**
    ```bash
    cmake --build .
    ```
    This will produce the shared library (`libyini.so`, `yini.dll`, or `libyini.dylib`) in the `build` directory.

## C# Interoperability

The shared library can be called from C# using P/Invoke. A sample wrapper class is provided in `csharp/Yini.cs`. To use it:

1.  Place the compiled shared library (e.g., `yini.dll`) next to your C# executable.
2.  Add `Yini.cs` to your C# project.
3.  You can now call the static methods in the `Yini` class to load and read values from your `.yini` files. See the example usage in the comments of `Yini.cs`.
