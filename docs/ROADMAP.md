# Future Development Roadmap

## 1. Feature Completeness
- **Dyna Type:** Implement `Dyna(value)` dynamic type if required by updated specs.
- **YMETA Generation:** Implement a build step to generate `.ymeta` cache files (separate from `.ybin` cooked assets) containing metadata for fast loading/reflection.
- **Struct Optimization:** Differentiate between `Struct` (fixed schema, high perf) and `Map` (dynamic keys) in the internal representation for optimization.

## 2. Tooling Integration
- **LSP Server:** Adapt the current Compiler/Validator logic into a Language Server Protocol (LSP) implementation. This will power the VSCode extension with real-time error reporting, autocomplete (based on schemas), and hover information.
- **VSCode Extension:** Integrate the C# LSP with the existing VSCode client.

## 3. Performance
- **Zero-Allocation Parsing:** Investigate `Span<T>` and `ref struct` optimizations for the Lexer to reduce memory pressure during large builds.
- **Parallel Compilation:** Update `Yini.CLI` to process multiple files in parallel for large projects.

## 4. Ecosystem
- **Unity/Godot Plugins:** Create specific bindings for game engines to load `.ybin` assets directly into engine-native objects.
- **Nuget Package:** Publish `Yini.Core` to NuGet for easy consumption by other .NET tools.
