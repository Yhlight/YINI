# YINI Project Improvement Suggestions

## 1. Introduction

The YINI project is in an excellent state. The codebase is robust, well-engineered, and fully compliant with the `YINI.md` specification. The following suggestions are not critical bug fixes but rather forward-looking enhancements aimed at improving the developer experience and expanding the capabilities of the YINI ecosystem.

## 2. Proposed Enhancements

### 2.1. Expand C# API for Complex Types

*   **Description**: The C# `YiniConfig.SetValue` API is currently limited to primitive types (int, double, bool, string). There is no direct way to create or modify complex types like arrays or maps.
*   **Rationale**: Adding support for setting complex types would significantly improve the ergonomics of creating new YINI configurations from scratch in C#. It would allow developers to work with familiar C# collection types.
*   **Proposed Action**:
    1.  Create new C++ interop functions to handle the creation and population of arrays and maps (e.g., `yini_create_array`, `yini_add_to_array`, `yini_set_array`, `yini_create_map`, etc.).
    2.  Add new `SetValue` overloads to the C# `YiniConfig` class that accept `IEnumerable<T>` for arrays and `IDictionary<string, object>` for maps.
    3.  Add corresponding unit tests to `csharp/tests/Yini.Core.Tests/WriteTests.cs` to verify the new functionality.

### 2.2. Add a `.ybin` Decompiler to the CLI

*   **Description**: The `yini` CLI has a `cook` command to compile `.yini` files into the optimized `.ybin` format. However, there is no corresponding `decompile` command to inspect the contents of a `.ybin` file.
*   **Rationale**: A decompiler would be an invaluable tool for debugging. It would allow developers to verify the final, resolved key-value pairs that are baked into the binary asset, helping to diagnose issues related to inheritance, includes, or macros.
*   **Proposed Action**:
    1.  Implement a new `decompile` command in the C++ CLI (`src/CLI/main.cpp`).
    2.  This command would use the `YbinData` class to load a `.ybin` file, iterate through all the key-value pairs, and print them to the console or a file in a human-readable `.yini` format.
    3.  Add a new integration test to `tests/CLITests.cpp` to verify the decompiler's output.

### 2.3. Enhance Language Server with Semantic Diagnostics

*   **Description**: The language server currently provides diagnostics for syntax errors (from the Parser). However, it does not report semantic errors that are detected later in the compilation pipeline, such as circular inheritance (Resolver) or schema violations (Validator).
*   **Rationale**: Pushing these semantic diagnostics to the VSCode client would provide immediate feedback to the user, significantly improving the development experience and catching errors before the user even tries to compile or run their code.
*   **Proposed Action**:
    1.  Modify the main language server loop in `src/CLI/main.cpp` to run the full Resolver and Validator on the source code.
    2.  Wrap these calls in `try-catch` blocks. When an exception is caught, parse the error message to extract the relevant information (error message, line, column).
    3.  Send the extracted information back to the client using the `textDocument/publishDiagnostics` LSP notification.
    4.  Add a new test file, `tests/LSPTests.cpp`, to specifically test that the correct diagnostics are generated for common semantic errors.
