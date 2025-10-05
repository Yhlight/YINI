# YINI Project Architecture

This document provides a high-level overview of the YINI project's architecture, detailing its major components and how they interact.

## Guiding Principles

The architecture is designed around two core principles:

1.  **Performance:** The core parsing and evaluation logic is written in modern C++ for maximum performance, which is critical for applications like game development where configuration files can be large and accessed frequently.
2.  **Developer Experience:** The library provides a seamless and high-level experience for C# developers, with modern features like source-generated P/Invoke and data binding. The dedicated Language Server further enhances this by providing rich IDE support.

## Component Diagram

```
+-------------------------------------------------+
|              IDE (e.g., VS Code)                |
|  +-------------------------------------------+  |
|  |           YINI VSCode Extension           |  |
|  +-------------------------------------------+  |
+----------------------|--------------------------+
                       | (LSP)
+----------------------v--------------------------+
|      C# Language Server (LspServer.dll)         |
|  - OmniSharp.Extensions.LanguageServer          |
|  - DocumentManager (manages YiniManager/file)   |
+----------------------|--------------------------+
                       | (C# Wrapper)
+----------------------v--------------------------+
|           C# Wrapper (Yini.dll)                 |
|  - YiniManager, YiniValue classes               |
|  - Source-generated P/Invoke ([LibraryImport])  |
|  - Source-generated binding ([YiniBindable])    |
+----------------------|--------------------------+
                       | (C-API)
+----------------------v--------------------------+
|             C-API (libYini.so/Yini.dll)         |
|  - C-style functions (e.g., yini_manager_...)   |
|  - Handle-based (Yini_ManagerHandle)            |
+----------------------|--------------------------+
                       | (C++ Core)
+----------------------v--------------------------+
|             C++ Core (Static Library)           |
|  - Lexer, Parser, Interpreter                   |
|  - YiniManager, YiniValue (C++ implementations) |
+-------------------------------------------------+
```

## Core Components

### 1. C++ Core (`src/Core`, `src/Lexer`, `src/Parser`, `src/Interpreter`)

This is the heart of the library, responsible for all the heavy lifting. It's written in C++17 for performance and portability.

*   **Lexer (`src/Lexer`):** Takes a string of source code and converts it into a stream of tokens (e.g., `IDENTIFIER`, `EQUAL`, `STRING_LITERAL`).
*   **Parser (`src/Parser`):** Takes the stream of tokens and constructs an **Abstract Syntax Tree (AST)**. The AST is a tree representation of the code's structure, composed of statements (like `KeyValue`, `Section`) and expressions (like `Binary`, `Literal`).
*   **Interpreter (`src/Interpreter`):** Traverses the AST to evaluate expressions and resolve values. It handles complex logic like section inheritance, macro resolution, and cross-references (`@{...}`). It populates a set of resolved key-value maps that represent the final state of the configuration.
*   **YiniManager (`src/Core/YiniManager.cpp`):** This is the main entry point for the C++ library. It orchestrates the lexer, parser, and interpreter. It also manages file loading (including `[#include]` directives) and the non-destructive saving of dynamic values.

### 2. C-API Layer (`src/Interop/YiniCApi.cpp`)

This layer provides a stable, C-compatible interface to the C++ core. It is the bridge between the native C++ world and the managed .NET world.

*   **C-Style Functions:** It exposes all public functionality through C-style functions (e.g., `yini_manager_create`, `yini_manager_get_value`).
*   **Handle-Based:** It uses opaque pointers (`Yini_ManagerHandle`, `Yini_ValueHandle`) to manage the lifetime of C++ objects, hiding the C++ implementation details from consumers.
*   **Error Handling:** It provides a thread-safe mechanism (`yini_manager_get_last_error`) for retrieving detailed error messages.

### 3. C# Wrapper (`csharp/Yini`)

This is the public library that .NET developers consume (e.g., via the `Yini` NuGet package). Its primary goal is to provide a safe, idiomatic, and high-performance C# API.

*   **`YiniManager.cs`:** The main managed class that mirrors the functionality of the C++ `YiniManager`. It contains all the high-level methods for loading and querying YINI data.
*   **Source-Generated P/Invoke (`[LibraryImport]`):** Instead of manual `[DllImport]`, the project uses the modern `[LibraryImport]` attribute. At compile time, the .NET SDK generates optimized, low-overhead marshalling code to call the C-API functions.
*   **`YiniValue.cs`:** A managed wrapper around the native `Yini_ValueHandle`. It implements `IDisposable` to ensure that the underlying native resources are correctly freed.
*   **Source-Generated Data Binding (`csharp/Yini.SourceGenerator`):** To provide a high-performance alternative to reflection-based binding, a custom source generator creates a `BindFromYini` extension method for any class marked with `[YiniBindable]`. This generated code directly calls the `YiniManager` methods, avoiding all reflection overhead.

### 4. C# Language Server (`ide/LspServer`)

This component provides rich IDE support for `.yini` files. It's a standalone console application that communicates with editors like VS Code using the Language Server Protocol (LSP).

*   **`OmniSharp.Extensions.LanguageServer`:** It is built on this powerful .NET library, which handles the complexities of the LSP communication.
*   **`DocumentManager`:** To support multiple files being open at once, the server maintains a `DocumentManager`. This class holds a dictionary mapping a file's URI to its own dedicated `YiniManager` instance. This design ensures that diagnostics and lookups for one file do not interfere with another.
*   **Handlers (`TextDocumentHandler`, `HoverHandler`, `DefinitionHandler`):** These classes respond to specific LSP requests from the client (the editor).
    *   `TextDocumentHandler`: Manages document synchronization (open, change, close), triggers validation, and publishes diagnostics (errors and warnings). It also provides completion for macros.
    *   `HoverHandler`: Provides detailed information about a key when the user hovers over it.
    *   `DefinitionHandler`: Provides "Go to Definition" functionality for keys and macros.