# YINI Project Improvement Suggestions

## 1. Introduction

This document provides a set of actionable recommendations for improving the YINI project. These suggestions are based on the findings of the comprehensive audit detailed in `NEW_AUDIT_REPORT.md`. While the project is in an excellent state, these proposals aim to further enhance its robustness, usability, and feature set.

## 2. High-Impact Core Enhancements

### Suggestion 2.1: Enhance C++ Interop Error Handling

*   **Observation:** The current interop layer uses a `thread_local` string (`last_error_message`) to report errors. While functional, this approach can be fragile, especially in a multi-threaded C# environment where a new error on one thread could overwrite a previous error before it's read by another.
*   **Recommendation:**
    1.  Modify the `yini_create_from_file` function and other fallible API calls to accept an optional `char**` for the error message.
    2.  The C++ side would allocate a new string for the error message (e.g., with `new char[]`) and assign its pointer to the output parameter.
    3.  A corresponding `yini_free_error_string(char*)` function should be provided.
    4.  The C# `NativeMethods` wrapper would then be responsible for calling the free function.
*   **Benefit:** This makes error handling more explicit and thread-safe, ensuring that the caller who initiated an operation is the one who receives the corresponding error message.

### Suggestion 2.2: Integrate Validator into the Resolver/Loader Flow

*   **Observation:** In `yini_interop.cpp`, the validation step is currently commented out (`// Validator validator(...)`). While the validator itself is correctly implemented, it is not being invoked during the standard file loading process.
*   **Recommendation:**
    1.  Uncomment and fully integrate the `Validator` call within the `YINI::Config` constructor for `.yini` files.
    2.  Ensure that any validation errors are properly caught and propagated as exceptions through the interop boundary.
*   **Benefit:** This is a critical fix to ensure that all schemas are automatically enforced when a `.yini` file is loaded, which is a core feature of the language.

## 3. C# API and Bindings Improvements

### Suggestion 3.1: Implement Full Array Type Safety

*   **Observation:** The C# `ValueType` enum has distinct types for arrays (`ArrayInt`, `ArrayDouble`, etc.), but the `yini_get_type` function in C++ only infers the array's subtype from its first element. An array like `[1, "hello", 2]` would be incorrectly identified as `ArrayInt`.
*   **Recommendation:**
    1.  Modify the C++ `Resolver` to perform a type consistency check when creating an array. If an array contains mixed types (which is not allowed by the `YINI.md` spec for typed arrays), it should be a resolution-time error.
    2.  This ensures that by the time the interop layer is reached, an array is guaranteed to be homogenous, making the `yini_get_type` logic safe and reliable.
*   **Benefit:** This prevents runtime errors in the C# layer and enforces stricter adherence to the language specification.

### Suggestion 3.2: Expose Struct and Map Types to C#

*   **Observation:** The C# API currently does not have methods to access `struct` or `map` types, even though the core language supports them. The `ValueType` enum includes `Struct` and `Map`, but there are no corresponding `GetStruct` or `GetMap` methods in `YiniConfig.cs`.
*   **Recommendation:**
    1.  Implement new C++ interop functions to get the keys and values of a map or the single key-value pair of a struct.
    2.  Add corresponding methods to the C# `YiniConfig` class, potentially returning a `Dictionary<string, object?>` for maps and a `KeyValuePair<string, object?>` for structs.
*   **Benefit:** This would expose the full power of the YINI language to .NET developers, making the C# bindings feature-complete.

## 4. CLI Tooling Enhancements

### Suggestion 4.1: Add a "Validate" Command to the CLI

*   **Observation:** The CLI can validate a file implicitly by loading it, but there is no explicit command for just performing validation. A dedicated command would be useful for CI/CD pipelines.
*   **Recommendation:**
    1.  Add a new command, `yini validate <file.yini>`, to the CLI.
    2.  This command would perform the full load, parse, resolve, and validate flow, reporting any errors to the console and exiting with a non-zero status code on failure.
*   **Benefit:** Improves the utility of the CLI as a tool for build and integration scripts.

### Suggestion 4.2: Add a "Decompile" Command to the CLI

*   **Observation:** The CLI includes a `cook` command to create `.ybin` files, but there is no corresponding command to inspect the contents of a `.ybin` file.
*   **Recommendation:**
    1.  Implement a `yini decompile <file.ybin>` command.
    2.  This command would read the `.ybin` file, traverse its hash table, and print a human-readable representation of its contents, similar to the original `.yini` format.
*   **Benefit:** This would be an invaluable debugging tool, allowing developers to verify the contents of their cooked assets without having to write custom code.
