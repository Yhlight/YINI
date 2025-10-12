# Contributing to YINI

We welcome contributions from the community! Whether you're fixing a bug, adding a new feature, or improving documentation, your help is appreciated.

## Getting Started

1.  **Fork the repository** on GitHub.
2.  **Clone your fork** locally: `git clone https://github.com/YOUR-USERNAME/YINI.git`
3.  **Set up the development environment:**
    *   You will need a C++17 compatible compiler (like GCC, Clang, or MSVC).
    *   You will need CMake (version 3.14 or higher).
    *   You will need the .NET SDK (version 8.0 or higher).
4.  **Build the project:** Run `python3 build.py` from the root directory. This will build all the C++ components.

## Running Tests

Before submitting any changes, please ensure that all tests pass.

*   **To run all tests (C++ and C#):**
    ```bash
    python3 build.py && cd build && ctest --output-on-failure && cd .. && dotnet test csharp/tests/Yini.Core.Tests
    ```

## Submitting a Pull Request

1.  Create a new branch for your changes: `git checkout -b feature/my-new-feature`
2.  Make your changes and commit them with a clear and descriptive commit message.
3.  Push your branch to your fork: `git push origin feature/my-new-feature`
4.  Open a pull request on the main YINI repository.

Thank you for contributing!
