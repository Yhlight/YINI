# YINI Project Improvement Suggestions - 2025-10-23

## 1. Overview

This document provides a list of actionable suggestions for improving the YINI project, based on the findings of the audit report.

## 2. C++ Core

*   **Lexer:**
    *   Add explicit token types for section headers, cross-section references, and environment variables to improve the clarity and robustness of the parser.
*   **Parser:**
    *   Refactor the schema validation parsing logic to be more token-based and less reliant on string manipulation.

## 3. C# Bindings

*   **Feature Completeness:**
    *   Add support for `color` and `coord` data types.
    *   Implement support for dynamic `Dyna()` values.
    *   Expose the schema validation and macro features to the C# API.

## 4. Test Suite

*   **Lexer:**
    *   Add tests for hex color codes (`#RRGGBB`).
    *   Add tests for arithmetic operators (`*`, `/`, `%`).
    *   Add tests for special characters (`@`, `$`, `~`, `?`).
    *   Add a test for the `+=` operator.
*   **Parser:**
    *   Add tests for set literals (`(1, 2, 3)`).
    *   Add tests to explicitly check the distinction between structs (`{key: value}`) and maps (`{key: value,}`).
    *   Add tests for `color` and `coord` literals.
    *   Add tests for arithmetic expressions.
    *   Add tests for environment variable references (`${ENV_VAR}`).
    *   Add tests for cross-section references (`@{Section.key}`).
    *   Add a test for the `[#include]` directive.
    *   Add tests for the `[#schema]` directive.

## 5. General

*   **Continuous Integration:**
    *   Set up a continuous integration (CI) pipeline to automatically build and test the project on every commit. This will help to catch regressions and ensure a high level of code quality.
*   **Documentation:**
    *   Expand the documentation to include more examples and tutorials for each of the language features.
    *   Add documentation for the C# API, explaining how to use the bindings to integrate YINI with .NET applications.
