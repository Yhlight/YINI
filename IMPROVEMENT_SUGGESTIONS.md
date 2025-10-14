# YINI Project: Improvement Suggestions

This document provides a list of actionable suggestions for improving the YINI project, based on the findings of the recent audit.

## 1. C++ Core

### 1.1. Enhance Test Coverage for Environment Variables

-   **Observation**: The `ResolverTests.cpp` file explicitly notes that tests for environment variables (`${VAR_NAME}`) are currently skipped.
-   **Suggestion**: Implement a suite of tests for this feature. This could involve setting environment variables within the test runner's process before executing the test cases.
-   **Benefit**: This would increase confidence in the correctness of this feature and prevent future regressions.

### 1.2. Clarify `list()` vs. `array()`

-   **Observation**: The `YINI.md` specification introduces both `list()` and `array()` syntax, but the `Resolver` currently treats both as `YiniArray`.
-   **Suggestion**: Formally define the semantic difference between `list` and `array` in `YINI.md`. If there is no difference, consider deprecating one of the syntaxes to reduce redundancy. If there is a difference, implement it in the C++ core and C# bindings.
-   **Benefit**: This would improve the clarity and consistency of the language.

## 2. C# Bindings

### 2.1. Expand Test Coverage for Complex Types

-   **Observation**: The C# tests in `Yini.Core.Tests` provide good coverage for primitive types but are less comprehensive for complex types like arrays, maps, and structs.
-   **Suggestion**: Add more detailed tests for these types. This should include tests for empty collections, collections of different primitive types, and nested collections (if applicable).
-   **Benefit**: This would increase the robustness of the C# API and provide better examples of how to use these features.

## 3. Developer Experience

### 3.1. Improve `.ybin` Decompiler Output

-   **Observation**: The `decompile` command is a valuable tool, but its output is a simple, flat list of key-value pairs.
-   **Suggestion**: Enhance the decompiler to reconstruct the original `.yini` structure as closely as possible, including section headers (`[Section]`).
-   **Benefit**: This would make the output more readable and easier to compare with the original `.yini` files.

### 3.2. Provide Real-time Semantic Diagnostics in VSCode

-   **Observation**: The VSCode extension currently provides diagnostics for syntax errors but not for semantic errors (e.g., schema violations, circular dependencies).
-   **Suggestion**: Enhance the C++ language server to perform full resolution and validation on file changes and report any semantic errors as diagnostics.
-   **Benefit**: This would provide a much richer and more helpful development experience for YINI users.