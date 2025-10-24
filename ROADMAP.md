# YINI Project Roadmap

This document outlines the development roadmap for the YINI project. It is intended to be a living document that is updated as the project progresses.

## Phase 1: Core Language and Tooling

### C++ Core
- [ ] **Lexer:**
    - [ ] Add explicit token types for section headers, cross-section references, and environment variables.
- [ ] **Parser:**
    - [ ] Refactor the schema validation parsing logic to be more token-based.

### Test Suite
- [ ] **Lexer:**
    - [ ] Add tests for hex color codes (`#RRGGBB`).
    - [ ] Add tests for arithmetic operators (`*`, `/`, `%`).
    - [ ] Add tests for special characters (`@`, `$`, `~`, `?`).
    - [ ] Add a test for the `+=` operator.
- [ ] **Parser:**
    - [ ] Add tests for set literals (`(1, 2, 3)`).
    - [ ] Add tests for the distinction between structs and maps.
    - [ ] Add tests for `color` and `coord` literals.
    - [ ] Add tests for arithmetic expressions.
    - [ ] Add tests for environment variable references (`${ENV_VAR}`).
    - [ ] Add tests for cross-section references (`@{Section.key}`).
    - [ ] Add a test for the `[#include]` directive.
    - [ ] Add tests for the `[#schema]` directive.

## Phase 2: C# Bindings and Interoperability

### C# Bindings
- [ ] **Feature Completeness:**
    - [ ] Add support for `color` and `coord` data types.
    - [ ] Design and implement a powerful `Dyna()` API for dynamic values.
    - [ ] Expose the schema validation and macro features to the C# API.

## Phase 3: Developer Experience

### Documentation
- [ ] Expand the documentation with more examples and tutorials.
- [ ] Add comprehensive documentation for the C# API.

### Continuous Integration
- [ ] Set up a CI pipeline to automate builds and tests.
