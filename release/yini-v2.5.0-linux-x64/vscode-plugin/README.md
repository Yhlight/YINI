# YINI Language Support for Visual Studio Code

Official VSCode extension for the YINI configuration language.

## Features

### ✅ Current Features (v2.0.0)

- **Syntax Highlighting** - Full YINI syntax highlighting
- **Auto-completion** - Smart completions for keywords and references
- **Error Detection** - Real-time syntax error checking
- **Code Navigation** - Jump to definitions
- **Hover Information** - View value details on hover

### 🎨 Syntax Support

- Comments (`//` and `/* */`)
- Sections (`[SectionName]`)
- Inheritance (`: Base1, Base2`)
- Macro definitions (`[#define]`)
- File includes (`[#include]`)
- Schema validation (`[#schema]`)
- All 12 YINI data types
- Macro references (`@name`)
- Cross-section references (`@{Section.key}`)
- Environment variables (`${VAR}`)
- Dynamic values (`Dyna()`)

## Installation

### From Source

1. **Build YINI LSP Server**:
   ```bash
   cd /path/to/yini
   python3 build.py --clean
   ```

2. **Install LSP Server** (make it available in PATH):
   ```bash
   sudo cp build/bin/yini_lsp /usr/local/bin/
   # OR set YINI_LSP_PATH environment variable
   export YINI_LSP_PATH=/path/to/yini/build/bin/yini_lsp
   ```

3. **Install VSCode Extension**:
   ```bash
   cd vscode-plugin
   npm install
   npm run compile  # If using TypeScript
   ```

4. **Package and Install**:
   ```bash
   npm install -g @vscode/vsce
   vsce package
   code --install-extension yini-language-support-2.0.0.vsix
   ```

## Configuration

### Settings

Access via: `File > Preferences > Settings > Extensions > YINI`

- **`yini.lsp.path`** - Path to YINI LSP server executable
  - Default: `"yini_lsp"` (uses PATH)
  - Example: `"/usr/local/bin/yini_lsp"`

- **`yini.trace.server`** - LSP communication tracing
  - Default: `"off"`
  - Options: `"off"`, `"messages"`, `"verbose"`

### Example settings.json

```json
{
    "yini.lsp.path": "/workspace/build/bin/yini_lsp",
    "yini.trace.server": "messages"
}
```

## Usage

### Syntax Highlighting

Open any `.yini` or `.YINI` file - syntax highlighting activates automatically.

### Auto-completion

- Type `[#` to see directive suggestions (`[#define]`, `[#include]`, `[#schema]`)
- Type `@` to see macro completions
- Type `@{` to see section completions

### Error Detection

Syntax errors appear with red squiggly lines in real-time.

### Go to Definition

- Right-click on `@macro_name` → Go to Definition
- Or press `F12` on a reference

### Hover Information

- Hover over any value to see type information
- Hover over macro references to see actual values

## Language Features

### Comments
```yini
// Single line comment
/* Multi-line
   comment */
```

### Sections and Inheritance
```yini
[Base]
key1 = value1

[Derived] : Base
key2 = value2
```

### Macro Definitions
```yini
[#define]
WIDTH = 1920
HEIGHT = 1080

[Graphics]
screen_width = @WIDTH
screen_height = @HEIGHT
```

### Cross-Section References
```yini
[Config]
width = 1920

[UI]
panel_width = @{Config.width}
```

### Data Types
- Integers: `123`
- Floats: `3.14`
- Booleans: `true`, `false`
- Strings: `"text"`
- Arrays: `[1, 2, 3]`
- Maps: `{key: value}`
- Colors: `#FF0000`, `Color(255, 0, 0)`
- Coordinates: `Coord(100, 200)`
- And more...

## Troubleshooting

### LSP Server Not Starting

1. **Check LSP server path**:
   ```bash
   which yini_lsp
   # Or check your settings
   ```

2. **Verify LSP server works**:
   ```bash
   yini_lsp --version  # Should start and wait for input
   # Press Ctrl+C to exit
   ```

3. **Check VSCode Output**:
   - View > Output
   - Select "YINI Language Server" from dropdown

### No Syntax Highlighting

1. Ensure file extension is `.yini` or `.YINI`
2. Reload VSCode: `Ctrl+Shift+P` → "Reload Window"

### No Auto-completion

1. Verify LSP server is running (check Output panel)
2. Check settings: `yini.lsp.path`
3. Try restarting VSCode

## Development

### Building from Source

```bash
cd vscode-plugin
npm install
npm run compile
```

### Testing

```bash
npm test
```

### Packaging

```bash
npm install -g @vscode/vsce
vsce package
```

## Requirements

- **Visual Studio Code** 1.60.0 or higher
- **YINI LSP Server** (yini_lsp executable)
- **Node.js** 16.x or higher (for extension only, not for LSP server)

## Technical Details

- **Language Server**: C++17 native implementation
- **Protocol**: LSP 3.17
- **Communication**: JSON-RPC over stdio
- **Performance**: <50ms response time

## Links

- [YINI Language Specification](../YINI.md)
- [YINI GitHub Repository](https://github.com/yourusername/yini)
- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)

## License

MIT License - See LICENSE file for details

## Changelog

### 2.0.0 (2025-10-06)
- ✨ Added C++ native LSP server
- ✨ Real-time syntax error detection
- ✨ Auto-completion support
- ✨ Hover information
- ✨ Go to definition
- 🔧 Improved language configuration

### 1.0.0
- ✅ Initial release with syntax highlighting
- ✅ Basic language configuration

## Support

For issues and feature requests, please visit the GitHub repository.
