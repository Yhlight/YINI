## YINI Language Support for VSCode

Features:
- Syntax highlighting for `.yini`/`.YINI`
- Syntax checking via `yini_cli check`
- Basic code completions and snippets
- Compile current file to `.ymeta`
- Decompile `.ymeta` to a new editor tab
- Built-in CLI terminal command
- Right-click to open bundled docs (`YINI.md`)

Commands:
- YINI: Compile Current File
- YINI: Decompile Current YMETA
- YINI: Open Official Docs
- YINI: Open CLI Terminal

Notes:
- The extension looks for `yini_cli` in the workspace root or `build/`. If not found, ensure it is built and available.

