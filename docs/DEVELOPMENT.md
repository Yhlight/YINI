# YINI Development Guide

## Architecture Overview

### State Machine + Strategy Pattern

The YINI project follows a state machine architecture with strategy pattern for extensibility.

#### Lexer State Machine

The lexer uses the State pattern for tokenization:

```
DefaultState ──┐
               ├─> IdentifierState
               ├─> NumberState
               ├─> StringState
               ├─> CommentState
               ├─> BlockCommentState
               ├─> SectionState
               └─> ReferenceState
```

Each state is responsible for:
- Processing characters
- Building token buffers
- Transitioning to other states
- Emitting tokens

**Key Design Decisions:**
1. **Unget Mechanism**: When transitioning states, we use `unget()` to put back characters that should be reprocessed by the next state
2. **Buffer Management**: Each state manages a shared buffer for building tokens
3. **EOF Handling**: Special handling for end-of-file to emit pending tokens

### Type System

The core type system uses `std::variant` for type-safe value storage:

```cpp
using ValueVariant = std::variant<
    std::monostate,           // Null
    long long,                // Integer
    double,                   // Float
    bool,                     // Boolean
    std::string,              // String
    std::vector<...>,         // Array/List
    std::map<...>,            // Map
    Color,                    // Color
    Coord,                    // Coord
    Path                      // Path
>;
```

## Test-Driven Development

All features follow TDD:

1. **Write Test**: Define expected behavior
2. **Red**: Test fails (not implemented)
3. **Green**: Minimal implementation to pass
4. **Refactor**: Improve code quality

### Running Tests

```bash
# All tests
python3 build.py test

# Specific test suite
./build/bin/test_lexer
./build/bin/test_core

# With verbose output
ctest --verbose
```

### Adding New Tests

1. Edit appropriate test file in `tests/`
2. Use the test framework:
   ```cpp
   TEST(MyNewTest)
   {
       // Arrange
       yini::Lexer lexer("test input");
       
       // Act
       auto tokens = lexer.tokenize();
       
       // Assert
       ASSERT_EQ(tokens.size(), 2);
       ASSERT_TRUE(tokens[0].is(TokenType::Identifier));
   }
   ```
3. Build and run tests

## Adding New Features

### Adding a New Token Type

1. Add to `TokenType` enum in `src/Lexer/Token.h`
2. Update `token_type_to_string()` in `src/Lexer/Token.cpp`
3. Add tokenization logic in appropriate `LexerState`
4. Write tests in `tests/test_lexer.cpp`

### Adding a New Value Type

1. Add to `ValueType` enum in `src/Core/Types.h`
2. Define the type structure
3. Update `ValueVariant` in `src/Core/Value.h`
4. Implement getters/setters
5. Write tests in `tests/test_core.cpp`

### Adding a New Lexer State

1. Create state class inheriting from `LexerState`
2. Implement `process()` method
3. Add state transition logic in `DefaultState` or other states
4. Write comprehensive tests

Example:
```cpp
class MyNewState : public LexerState
{
public:
    std::unique_ptr<LexerState> process(Lexer& lexer, char ch) override
    {
        // State logic here
        if (/* condition */)
        {
            lexer.emitToken(TokenType::MyToken);
            return std::make_unique<DefaultState>();
        }
        return nullptr; // Stay in current state
    }
    
    std::string getName() const override { return "MyNew"; }
};
```

## Code Style Guide

### Naming Conventions

```cpp
// Functions and basic type variables
int calculate_sum(int value_a, int value_b);

// Class member variables (basic types)
class MyClass
{
private:
    int count_;
    float distance_;
};

// Class member variables (non-basic types)
class MyClass
{
private:
    std::string myString;
    std::vector<int> dataList;
};

// Class member functions
class MyClass
{
public:
    void calculateTotal();
    int getValue() const;
};

// Data structures and classes
class MyDataStructure { };
struct MyStruct { };
```

### Brace Style (Allman)

```cpp
if (condition)
{
    // Code here
}
else
{
    // Code here
}

class MyClass
{
public:
    void myMethod()
    {
        // Code here
    }
};
```

### Comments

```cpp
// Single-line comments for brief explanations

/*
 * Multi-line comments for detailed explanations
 * Used for complex logic or documentation
 */

/// Documentation comments (planned for Doxygen)
```

## Build System

### CMake Structure

- `CMakeLists.txt`: Main configuration
- `tests/CMakeLists.txt`: Test configuration

### Python Build Script

The `build.py` script provides:
- Automated configuration
- Build management
- Test execution
- Clean operations

### Adding New Build Targets

Edit `CMakeLists.txt`:
```cmake
add_executable(my_new_target src/my_file.cpp)
target_link_libraries(my_new_target yini_core yini_lexer)
```

## Debugging

### VSCode Integration

Use the provided launch configurations:
- Debug YINI CLI
- Debug Lexer Tests
- Debug Core Tests

### Common Issues

**Issue**: Lexer creating empty tokens
- Check state transitions
- Verify `unget()` usage
- Ensure buffer is cleared after emission

**Issue**: Tests failing after refactor
- Check for state machine changes
- Verify token types haven't changed
- Review buffer management

**Issue**: Build errors with unused variables
- Add `(void)variable;` to suppress warnings temporarily
- Implement the feature properly as soon as possible

## Performance Considerations

### Current Optimizations
- State machine avoids regex overhead
- String buffer reuse
- Single-pass tokenization

### Future Optimizations (Planned)
- Memory pooling for tokens
- Lazy evaluation for values
- YMETA file caching

## Contributing Workflow

1. Create feature branch
2. Write failing tests (TDD)
3. Implement minimal code
4. Ensure all tests pass
5. Refactor if needed
6. Update documentation
7. Create pull request

## Next Steps

### Parser Implementation

The parser will:
1. Consume tokens from lexer
2. Build Abstract Syntax Tree (AST)
3. Validate structure
4. Resolve references
5. Evaluate expressions

Planned structure:
```
ParserState ──┐
              ├─> SectionParseState
              ├─> KeyValueParseState
              ├─> ExpressionParseState
              └─> SchemaValidateState
```

### C# Bindings

P/Invoke interface for:
- Creating lexer instances
- Tokenizing input
- Parsing YINI files
- Accessing values

### VSCode Extension

Features:
- Syntax highlighting
- Auto-completion
- Error detection
- Format document
- Go to definition (cross-references)
