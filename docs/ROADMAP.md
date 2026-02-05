# Future Development Roadmap

## 1. Tooling Integration & UI
- **VSCode Extension UI:** Integrate the C# `Yini.LSP` executable with the VSCode extension client. Add syntax highlighting grammar (TextMate) to match the LSP features.
- **Game Engine Editors:** Create Editor Windows for Unity and Godot to visually edit `.ybin` files or inspect loaded configs.

## 2. Advanced Analysis
- **Roslyn Analyzer:** Create a NuGet package `Yini.Analyzer` that runs `Yini.Validator` during the C# build process if `.yini` files are included in the project, preventing build on invalid config.
- **Unused Key Detection:** Analyze codebases (C#) to find `yini["key"]` usages and report config keys that are defined but never used in code.

## 3. Performance & Scale
- **Incremental Compilation:** Update `Yini.CLI` to only rebuild `.ybin` files if the source `.yini` (or its includes) has changed since last build.
- **Memory Optimization:** Fully migrate `Parser` to use `LexerFast` and `Span<T>` to reduce GC pressure for massive configuration files (100MB+).

## 4. Ecosystem Growth
- **Localization Support:** Add native support for localization keys (e.g., `text = @i18n:welcome`) within the compiler resolution pipeline.
- **Remote Config:** Implement a feature to patch `.ybin` files at runtime from a remote server delta.
