# YINI
现代化的INI，使用于游戏开发

## Getting Started

YINI is a modern, feature-rich configuration file format designed for game development. This repository contains the source code for the YINI C++ library and its command-line interface (CLI).

For a detailed description of the YINI language features, please see `YINI.md`.

## Building the Project

The project uses CMake for building. You will need a C++ compiler that supports C++17 (like GCC, Clang, or MSVC) and CMake (version 3.10 or higher).

### Dependencies

The project has one external dependency for its test suite:
- **Google Test**: CMake will automatically download and configure this dependency if it is not found on your system.

### Build Steps

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/your-username/yini.git
    cd yini
    ```

2.  **Create and navigate to a build directory:**
    ```sh
    mkdir build
    cd build
    ```

3.  **Run CMake to configure the project:**
    ```sh
    cmake ..
    ```

4.  **Compile the source code:**
    ```sh
    cmake --build .
    ```

This process will generate the following artifacts in the `build` directory:
- `libYINI.a`: The static library.
- `yini-cli`: The command-line interface executable.
- `yini_tests`: The test suite runner.

## Using the CLI

The `yini-cli` tool provides an interactive shell for working with `.yini` and `.ymeta` files.

You can run it from the `build` directory:
```sh
./yini-cli
```

### CLI Commands

The CLI supports the following commands:

-   `check <filepath>`: Checks the syntax of a `.yini` file and reports any errors.
    ```
    > check path/to/your/config.yini
    Syntax OK: path/to/your/config.yini
    ```

-   `compile <filepath>`: Compiles a `.yini` file into its binary `.ymeta` representation for faster loading in an application.
    ```
    > compile path/to/your/config.yini
    Successfully compiled path/to/your/config.yini to .ymeta
    ```

-   `decompile <filepath>`: Decompiles a `.ymeta` file back into a human-readable text format, printing the result to the console.
    ```
    > decompile path/to/your/config.ymeta
    --- Decompilation of path/to/your/config.ymeta ---
    ...
    --- End of Decompilation ---
    ```

-   `help`: Shows the list of available commands.

-   `exit`: Exits the interactive CLI.