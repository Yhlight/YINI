# YINI - Modern Configuration Language

YINI is a modern configuration language designed for game development, built with C++17. It extends the traditional INI file format with powerful features like inheritance, dynamic values, type system, and more.

## Features

### Implemented ✅
- **Lexer (Tokenizer)** - Full state machine-based lexical analysis
  - Comments (`//` and `/* */`)
  - Sections (`[Config]`, `[#define]`, `[#include]`, `[#schema]`)
  - Multiple value types (integers, floats, booleans, strings)
  - Operators (`+`, `-`, `*`, `/`, `%`, `=`, `+=`)
  - References (`@macro`, `${ENV_VAR}`, `@{Section.key}`)
  - All delimiters and special characters
  
- **Core Type System**
  - Value types: Integer, Float, Boolean, String, Color, Coord, Path
  - Type conversion and validation
  - Dynamic value support (`Dyna()`)

- **Build System**
  - CMake configuration
  - Python build script for automation
  - Unit testing framework
  - Cross-platform compilation

### Planned 🚧
- **Parser** - AST generation from tokens
  - Section parsing with inheritance
  - Expression evaluation
  - Schema validation
  - File inclusion
  
- **YMETA** - Metadata file generation
  - Caching system
  - Dynamic value persistence
  
- **CLI Tool** - Command-line interface
  - Compile/decompile YMETA files
  - Syntax checking
  - Interactive mode
  
- **C# Bindings** - P/Invoke API for cross-platform C# support

- **VSCode Extension** - Language support plugin

## Building

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.15 or higher
- Python 3.x

### Build Commands

```bash
# Configure the project
python3 build.py configure

# Build all targets
python3 build.py build

# Run tests
python3 build.py test

# Clean build directory
python3 build.py clean

# Do everything (clean, configure, build, test)
python3 build.py all
```

### Build Options

```bash
# Debug build (default)
python3 build.py configure --build-type Debug

# Release build
python3 build.py configure --build-type Release

# Build specific target
python3 build.py build --target yini
```

## Project Structure

```
YINI/
├── src/
│   ├── Core/          # Core type system and values
│   ├── Lexer/         # Tokenization (state machine)
│   ├── Parser/        # AST generation (TODO)
│   ├── CLI/           # Command-line tool (TODO)
│   └── API/           # C# P/Invoke bindings (TODO)
├── tests/             # Unit tests
├── docs/              # Documentation
├── build.py           # Python build script
├── CMakeLists.txt     # CMake configuration
└── YINI.md            # Language specification
```

## Language Features

See [YINI.md](YINI.md) for the complete language specification.

### Quick Example

```yini
[#define]
width = 1920
height = 1080

[Config]
title = "My Game"
fullscreen = true

[Graphics] : Config
resolution = @{Config.width} * @{Config.height}
fps = Dyna(60)
color = #FF5733
position = Coord(100, 200, 50)

[#include]
+= ui_config.yini
+= audio_config.yini
```

## Architecture

### Design Patterns
- **State Machine**: Lexer uses state pattern for tokenization
- **Strategy Pattern**: Value type handling and parsing strategies
- **Visitor Pattern**: (Planned) AST traversal

### Coding Style
- **Naming Conventions**:
  - Basic types, regular functions: snake_case
  - Class member variables (basic types): snake_case
  - Class member variables (non-basic types): camelCase
  - Class member functions: camelCase
  - Data structures/classes: PascalCase

- **Brace Style**: Allman

## Testing

All components are developed using Test-Driven Development (TDD):

```bash
# Run all tests
python3 build.py test

# Run specific test
./build/bin/test_lexer
./build/bin/test_core
```

Current test coverage:
- ✅ Core type system (100%)
- ✅ Lexer tokenization (100%)
- 🚧 Parser (0% - not yet implemented)

## Contributing

This project follows incremental TDD development. When adding features:

1. Write tests first
2. Implement minimal code to pass tests
3. Refactor while keeping tests green
4. Update documentation

## License

See [LICENSE](LICENSE) for details.

## Roadmap

### Phase 1: Lexer ✅ (Complete)
- [x] Token definitions
- [x] State machine architecture
- [x] Basic tokenization
- [x] Comments and operators
- [x] References and special sections
- [x] All value types

### Phase 2: Parser 🚧 (In Progress)
- [ ] Section parsing
- [ ] Key-value pairs
- [ ] Inheritance resolution
- [ ] Expression evaluation
- [ ] Macro expansion
- [ ] Schema validation

### Phase 3: Runtime
- [ ] YMETA file generation
- [ ] Dynamic value tracking
- [ ] File inclusion
- [ ] Environment variable substitution

### Phase 4: Tooling
- [ ] CLI implementation
- [ ] C# bindings
- [ ] VSCode extension

### Phase 5: Optimization
- [ ] Performance profiling
- [ ] Memory optimization
- [ ] Caching improvements
