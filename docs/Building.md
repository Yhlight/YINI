# Building and Integrating YINI

This guide provides instructions on how to build and integrate the YINI library into your project.

## Prerequisites

*   **CMake:** You will need to have CMake installed on your system.
*   **C++17 Compiler:** You will need a C++17 compatible compiler, such as GCC, Clang, or MSVC.
*   **.NET 8.0 SDK (optional):** If you want to use the C# wrapper, you will need to have the .NET 8.0 SDK installed.

## Building

To build the YINI library, follow these steps:

1.  **Clone the repository:** Clone the YINI repository to your local machine.
2.  **Run the build script:** Run the `build.py` script to build the project.

```bash
python3 build.py
```

This will create a `build` directory containing the compiled library and executables.

## Integrating

To integrate the YINI library into your project, you will need to:

1.  **Include the YINI headers:** Add the `src` directory to your include path.
2.  **Link against the YINI library:** Link against the `libYini.so` (on Linux) or `Yini.dll` (on Windows) library.

## C# Integration

To use the YINI C# wrapper, you will need to:

1.  **Add a reference to the Yini.dll:** Add a reference to the `Yini.dll` in your C# project.
2.  **Copy the native library:** Copy the `libYini.so` or `Yini.dll` to your project's output directory.