# Future Development Roadmap

## 1. Tooling Integration & UI
- **VSCode Extension UI:** Integrate the C# `Yini.LSP` executable with the VSCode extension client. Add syntax highlighting grammar (TextMate) to match the LSP features.
- **Game Engine Editors:** Create Editor Windows for Unity and Godot to visually edit `.ybin` files or inspect loaded configs.

## 2. Advanced Analysis
- **Roslyn Analyzer:** Create a NuGet package `Yini.Analyzer` that runs `Yini.Validator` during the C# build process if `.yini` files are included in the project, preventing build on invalid config.
- **Unused Key Detection:** Analyze codebases (C#) to find `yini["key"]` usages and report config keys that are defined but never used in code.

## 3. Performance & Scale
- **Bytecode VM for Dyna:** Currently `Dyna` expressions are evaluated by walking the AST. For per-frame evaluation in games, compiling these to a compact bytecode (or C# delegates) would significantly improve performance.
- **Memory Optimization:** Fully migrate `Parser` to use `LexerFast` and `Span<T>` to reduce GC pressure for massive configuration files (100MB+).

## 4. Ecosystem Growth
- **Remote Config:** Implement a feature to patch `.ybin` files at runtime from a remote server delta.
- **Strongly Typed Code Gen:** Enhance `gen-cs` to automatically generate C# `structs` or `ScriptableObjects` (Unity) / `Resources` (Godot) that map 1:1 with defined Schemas, removing string-based lookups entirely.

## Completed Items
- **Incremental Compilation:** `Yini.CLI` now supports `BuildCache` to skip unchanged files.
- **Localization Support:** Native support for `@i18n:key` reference resolution.
- **Evaluator:** Runtime AST evaluation for expressions and `Dyna` types.
