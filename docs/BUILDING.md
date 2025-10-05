# Building YINI from Source

This guide provides instructions for setting up a development environment to build the YINI project from source.

## Prerequisites

You will need the following tools and libraries installed on your system to build and test the entire project.

### 1. C++ Toolchain

A C++17 compliant compiler is required.

-   **On Debian/Ubuntu:**
    ```bash
    sudo apt-get update
    sudo apt-get install build-essential g++
    ```

### 2. CMake

CMake is used as the build system for the project. A version of 3.10 or higher is required.

-   **On Debian/Ubuntu:**
    ```bash
    sudo apt-get install cmake
    ```

### 3. .NET 8.0 SDK

The .NET 8.0 SDK is required for building the C# bindings, source generator, and language server.

-   **On Debian/Ubuntu:**
    Instructions for installing the .NET SDK can be found on the official Microsoft documentation. A typical installation involves adding the Microsoft package repository:
    ```bash
    # Download and install the Microsoft package signing key
    wget https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
    sudo dpkg -i packages-microsoft-prod.deb
    rm packages-microsoft-prod.deb

    # Install the SDK
    sudo apt-get update
    sudo apt-get install -y dotnet-sdk-8.0
    ```

### 4. Doxygen and Graphviz (Optional)

Doxygen and Graphviz are required to generate the C++ API documentation.

-   **On Debian/Ubuntu:**
    ```bash
    sudo apt-get install doxygen graphviz
    ```

## Building the Project

Once all prerequisites are installed, you can build the project using CMake:

1.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

2.  **Configure the project:**
    ```bash
    cmake ..
    ```

3.  **Build the project:**
    ```bash
    cmake --build .
    ```

This will build the C++ core library, the C# projects, and all tests.

## Running Tests

-   **C++ Tests:** From the `build` directory, run:
    ```bash
    ctest --output-on-failure
    ```
-   **C# Tests:** From the `csharp` directory, run:
    ```bash
    dotnet test
    ```

## Generating Documentation (Optional)

If you have Doxygen and Graphviz installed, you can generate the C++ API documentation by running the following command from the `build` directory:
```bash
cmake --build . --target doc
```
The output will be located in `docs/api/html`.