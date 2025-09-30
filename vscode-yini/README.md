# YINI Language Support for VS Code

This extension provides comprehensive language support for the YINI configuration file format. It aims to make writing and editing `.yini` files a fast, efficient, and error-free experience.

## Features

This extension bundles a powerful Language Server Protocol (LSP) server written in C++ to provide the following features:

*   **Rich Syntax Highlighting**: Accurate and detailed coloring for all YINI syntax constructs, including sections, keys, comments, strings, numbers, booleans, and special function calls like `Dyna()`, `List()`, `Coord()`, etc.

*   **Real-time Syntax Error Diagnostics**: Get immediate feedback as you type. The extension will underline syntax errors and provide descriptive messages on hover, helping you fix issues before they become problems.

*   **Intelligent Hover Information**: Hover over a key to see its fully serialized value, providing quick insight into complex data structures like maps and arrays.

*   **Context-Aware Autocompletion**:
    *   Get suggestions for known section names.
    *   Get suggestions for macro names when you type `@`.

## How It Works

This extension automatically starts the `yini-lsp` language server in the background. The server analyzes your `.yini` files as you type, providing the features listed above.

## Getting Started

1.  Install the extension from the VS Code Marketplace.
2.  Open any file with a `.yini` or `.YINI` extension.
3.  The language features will activate automatically.

---

**Enjoy a more productive YINI editing experience!**