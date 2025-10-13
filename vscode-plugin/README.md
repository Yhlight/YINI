# YINI Language Support for Visual Studio Code

Syntax highlighting and language support for YINI configuration files in Visual Studio Code.

## Features

- 🎨 **Syntax Highlighting**: Full syntax highlighting for YINI files
- 📝 **Auto-completion**: Bracket and quote auto-closing
- 💬 **Comment Support**: Line comments (`//`) and block comments (`/* */`)
- 📐 **Code Folding**: Fold sections for better code organization
- 🎯 **Smart Indentation**: Auto-indentation for sections

## Installation

### From VSIX (Recommended)

1. Download the `.vsix` file from releases
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Click the "..." menu and select "Install from VSIX..."
5. Select the downloaded `.vsix` file

### From Source

1. Clone the repository
2. Copy the `vscode-plugin` folder to your VS Code extensions directory:
   - **Windows**: `%USERPROFILE%\.vscode\extensions\`
   - **macOS**: `~/.vscode/extensions/`
   - **Linux**: `~/.vscode/extensions/`
3. Reload VS Code

## Supported Syntax

### Sections

```ini
[Config]
key = value

[Derived] : Base1, Base2
inherited = true
```

### Data Types

```ini
# Numbers
integer = 123
float = 3.14

# Booleans
flag = true

# Strings
text = "Hello, World!"

# Arrays
items = [1, 2, 3]
nested = [[1, 2], [3, 4]]

# Maps
settings = {width: 1920, height: 1080}

# Sets
values = (1, 2, 3)

# Colors
color1 = #FF0000
color2 = Color(255, 0, 0)

# Coordinates
pos2d = Coord(100, 200)
pos3d = Coord(100, 200, 300)

# Paths
file = Path("data/config.json")

# Lists
items = List(1, 2, 3)

# Dynamic values
score = Dyna(0)
```

### Directives

```ini
[#define]
WIDTH = 1920
HEIGHT = 1080

[#include]
+= "common.yini"
+= "platform.yini"

[#schema]
[Config]
width = !, int, =1920
```

### References

```ini
# Macro references
width = @WIDTH

# Cross-section references
ui_width = @{Graphics.width}

# Environment variables
home = ${HOME}
```

### Operators

```ini
half = @WIDTH / 2
total = 10 + 20 * 3
```

### Quick Register

```ini
[Registry]
+= "item1"
+= "item2"
+= "item3"
```

## Color Themes

The syntax highlighting works with all VS Code color themes. The following scopes are used:

- `entity.name.section.yini` - Section names
- `keyword.control.directive.yini` - Directives (#define, #include, etc.)
- `variable.other.key.yini` - Keys
- `constant.numeric.*` - Numbers
- `constant.language.boolean.yini` - Booleans
- `string.quoted.double.yini` - Strings
- `constant.other.color.hex.yini` - Hex colors
- `support.type.builtin.yini` - Built-in types
- `variable.other.reference.yini` - References
- `comment.line.yini` - Comments

## Configuration

Currently no additional configuration is needed. The extension works out of the box.

## Known Issues

- Advanced semantic highlighting not yet implemented
- No IntelliSense/autocomplete suggestions yet
- No real-time validation yet

These features are planned for future releases.

## Roadmap

- [ ] Language Server Protocol (LSP) support
- [ ] IntelliSense and autocomplete
- [ ] Real-time syntax validation
- [ ] Go to definition for references
- [ ] Hover information
- [ ] Snippet support
- [ ] Refactoring tools

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License

## Links

- [YINI Language Specification](../YINI.md)
- [GitHub Repository](https://github.com/yourusername/yini)
- [Issue Tracker](https://github.com/yourusername/yini/issues)

## Release Notes

### 1.0.0

- Initial release
- Basic syntax highlighting
- Comment support
- Auto-closing pairs
- Code folding
