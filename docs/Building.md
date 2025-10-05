# Building and Integrating YINI

This guide provides instructions on how to build the YINI library from source. For most .NET projects, the recommended approach is to use the official NuGet package.

## Prerequisites

*   **CMake (version 3.10 or higher):** Required for building the C++ core library.
*   **A C++17 compatible compiler:** (e.g., GCC, Clang, or MSVC).
*   **.NET 8.0 SDK (optional):** Required if you want to build the C# wrapper and run the tests.

## Building from Source

The YINI project now uses a unified CMake build system that handles both the C++ and C# components.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-repo/YINI.git
    cd YINI
    ```

2.  **Configure and build with CMake:**
    ```bash
    # Create a build directory
    mkdir build
    cd build

    # Configure the project
    cmake ..

    # Build both C++ and C# components
    cmake --build .
    ```

This will:
*   Compile the native C++ library (`libYini.so` on Linux, `Yini.dll` on Windows).
*   Build the C# wrapper (`Yini.dll`), the source generator, and the test project.
*   The native library will be automatically copied to the correct location for the C# wrapper to use.

## Integration

### C++ Projects

1.  **Include Headers:** Add the `src` directory of the YINI project to your include paths.
2.  **Link Library:** Link your project against the compiled YINI library.

If you installed YINI system-wide, you can use `find_package(Yini)` in your `CMakeLists.txt`:
```cmake
find_package(Yini REQUIRED)
target_link_libraries(your_target PRIVATE Yini::Yini)
```

### C# Projects (.NET)

The easiest way to use YINI in your .NET project is to add the NuGet package.

If you are building from source and want to use the local build output:

1.  **Add a Project Reference:** Add a reference to the `Yini.csproj` file located in the `csharp/Yini` directory.
    ```xml
    <ProjectReference Include="path/to/YINI/csharp/Yini/Yini.csproj" />
    ```
2.  **Build Your Project:** The YINI native library and its C# wrapper will be automatically included in your project's build output. The unified CMake build handles the details of placing the native libraries in the correct `runtimes` folder.