# Changelog

All notable changes to the YINI project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Future Plans
- **LSP Enhancements**: Implement `hover`, `go-to-definition`, `document-symbols`, and `find-references`.
- **Advanced Schema Validation**: More complex validation rules and better error reporting.
- **Performance Optimizations**: Profile and optimize parsing and memory usage.
- **Additional Language Bindings**: Python, Rust.

## [1.1.0-alpha] - 2025-10-06

This is a significant stabilization and refactoring release focused on improving correctness, testing, and error handling based on a comprehensive code review.

### Added
- **Robust Negative Testing**: Introduced a full suite of negative tests for both the Lexer and Parser using GoogleTest. The parser now correctly handles and reports on invalid syntax, unresolved references, circular references, and schema violations.
- **Enhanced Error Reporting**: The error reporting mechanism has been overhauled. It now provides accurate line and column numbers for semantic and validation errors, not just syntax errors.

### Changed
- **Testing Framework Modernization**: The entire test suite has been migrated from a basic `cassert` implementation to the industry-standard GoogleTest framework. This provides more robust assertions and significantly better diagnostic output on test failures.
- **Architectural Documentation**: Updated `README.md` and `YINI.md` to accurately describe the project's architecture as a "Recursive Descent Parser with a Pratt Parser for expressions," removing misleading references to "State Machine" and "Strategy" patterns.

### Fixed
- **Critical Type-Loss Bug**: Corrected a major bug where the specific types for `Path`, `List`, `Set`, and `Tuple` were lost during parsing and were incorrectly treated as generic strings or arrays. The `Value` class and `Parser` now fully preserve this semantic information as specified in `YINI.md`.
- **Parser Ambiguity**: Resolved a critical ambiguity in the parser regarding `()` syntax. The parser can now correctly distinguish between grouped arithmetic expressions (e.g., `(1 + 2)`), Tuples (e.g., `(1, "a")`), and Sets (e.g., `(1, 2, 3)`).
- **Schema Parsing**: Fixed a bug in the schema parsing logic that caused incorrect parsing of schema definitions.
- **Error Handling Logic**: Corrected multiple bugs in the error handling flow where specific, useful error messages were being overwritten by more generic ones.
- **Build & Deserialization**: Fixed a compilation error in the YMETA deserialization logic caused by recent changes to the `Value` class.

## [1.0.0] - 2025-10-06

### Added
- Complete Lexer implementation with state machine pattern
  - 20+ token types support
  - Comments support (// and /* */)
  - Intelligent hex color parsing
  - Full error reporting with line and column numbers
  - 15 unit test cases with 100% pass rate

- Complete Parser implementation with strategy pattern
  - Section parsing with inheritance support
  - Quick register operator (+=)
  - Arithmetic expressions with proper precedence
  - Macro definitions and references (#define, @name)
  - File inclusion (#include)
  - Environment variables (${NAME})
  - Cross-section references (@{Section.key}) framework
  - 11 unit test cases with 100% pass rate

- Rich data type system
  - Primitives: int64, double, bool, string
  - Collections: arrays, sets, maps
  - Special types: Color, Coord, Path, List, Dyna
  - Full support for all YINI value types

- CLI tool (yini_cli)
  - Interactive command loop
  - Commands: help, parse, check, compile, decompile, exit
  - Beautiful UI with box-drawing characters
  - Comprehensive error messages

- YMETA binary format
  - Efficient binary serialization
  - Magic number and version control
  - Full round-trip support (YINI ↔ YMETA)
  - Data integrity verification

- C# P/Invoke bindings
  - Complete C API wrapper
  - Type-safe C# classes
  - Automatic memory management (IDisposable)
  - Example code and documentation
  - Thread-safe design

- VSCode extension
  - Syntax highlighting (TextMate grammar)
  - Auto-closing pairs
  - Code folding
  - Smart indentation
  - Language configuration

- Build system
  - CMake configuration
  - Python build script (build.py)
  - Automated testing with CTest
  - Cross-platform support

- Documentation
  - Language specification (YINI.md)
  - Implementation summary
  - Project completion report
  - API documentation
  - C# binding guide
  - VSCode plugin guide
  - Multiple example files

### Technical Details
- C++17 standard
- State machine + Strategy pattern architecture
- TDD-driven development
- Zero compiler warnings (-Wall -Wextra -Wpedantic -Werror)
- Zero memory leaks (smart pointers)
- Allman bracket style
- Clear naming conventions

### Statistics
- Total lines of code: ~5,774
- Source files: 15 (.h + .cpp)
- Test files: 2
- Test cases: 26 (100% pass rate)
- Build time: ~3 seconds (full build)
- Test execution: <100ms

---

*For detailed changes, see the git commit history.*