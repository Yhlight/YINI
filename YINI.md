# YINI Language Specification

YINI is a modern configuration language designed to be a powerful and flexible replacement for traditional INI files, especially in game development contexts. It extends the simple key-value structure of INI with advanced features like data types, inheritance, and dynamic values.

## File Extension
YINI files use the `.yini` or `.YINI` extension.

## Basic Syntax

### Comments
YINI supports C-style comments:
-   **Single-line comments**: Start with `//` and continue to the end of the line.
-   **Block comments**: Start with `/*` and end with `*/`.

```yini
// This is a single-line comment.
key = value /* This is a block comment. */
```

### Sections
Configuration is organized into sections, denoted by a name enclosed in square brackets `[]`.

```yini
[Graphics]
width = 1920
height = 1080
```

### Key-Value Pairs
Within a section, configuration is defined as key-value pairs, separated by an equals sign `=`.

```yini
[Audio]
volume = 1.0
```

## Advanced Features

### Inheritance
A section can inherit from one or more parent sections, inheriting their key-value pairs. Values from later parents in the list will override earlier ones, and values in the child section will override all inherited values.

```yini
[BaseConfig]
quality = "high"

[Graphics] : BaseConfig
fullscreen = true
```

### Quick Registration
For creating lists of values without explicit keys, YINI provides a quick registration operator `+=`. The keys will be auto-incremented integers starting from 0 for each section.

```yini
[Registry]
+= "first_item"
+= "second_item" // key will be "1"
```

### Data Types
YINI supports a rich set of data types:
-   **Integer**: `123`
-   **Float**: `3.14`
-   **Boolean**: `true` or `false`
-   **String**: `"hello world"`
-   **Array**: `[1, 2, 3]` (A standard, indexable array)
-   **List**: `list(1, "two", 3.0)` (An explicitly declared list)
-   **Set**: `(1, "two", 3.0)` (A collection of unique values)
-   **Map**: `{key1: "value1", key2: 123}` (A collection of key-value pairs)
-   **Color**: Can be defined in hex (`#RRGGBB`) or RGB function style (`color(255, 192, 203)`).
-   **Coordinate**: `coord(10, 20)` for 2D or `coord(10, 20, 30)` for 3D.
-   **Path**: `path("/usr/local/bin")`

### Dynamic Values and YMETA Caching
YINI supports dynamic values that can be updated during runtime. These values are wrapped in the `Dyna()` function.

```yini
[Player]
health = Dyna(100)
```
When a YINI file is loaded, a corresponding `.ymeta` file is created to cache the state of dynamic values. This allows their state to persist across sessions. The `.ymeta` file also stores up to five previous values for each dynamic key as a backup.

### Arithmetic Operations
YINI supports arithmetic operations (`+`, `-`, `*`, `/`, `%`) on numeric values. Standard operator precedence is respected, and parentheses `()` can be used to control the order of evaluation.

```yini
[Calculations]
result = (10 + 5) * 2
```

## Directives
Directives are special sections that provide meta-functionality.

### Macro Definitions (`[#define]`)
Macros can be defined in a `[#define]` section and referenced elsewhere using the `@` symbol.

```yini
[#define]
default_width = 800

[Graphics]
width = @default_width
```

### File Includes (`[#include]`)
YINI files can include other YINI files to merge configurations. The `+=` operator is used to specify the paths to include.

```yini
[#include]
+= "base_settings.yini"
+= "user_overrides.yini"
```

### Schema Validation (`[#schema]`)
A schema can be defined to validate the structure and values of a configuration section.

```yini
[#schema]
[Graphics]
width = !, int, min=800, max=3840, =1920
height = !, int, min=600, max=2160, =1080
vsync = ?, bool, =true
```
**Validation Rules:**
-   **Required/Optional**: `!` (required), `?` (optional).
-   **Type**: `int`, `float`, `bool`, `string`, etc.
-   **Default Value**: `=value` (e.g., `=1920`).
-   **Range**: `min=value`, `max=value`.
-   **Empty Behavior**: `e` (error on empty), `~` (ignore on empty).

## References

### Cross-Section References
Values from other sections can be referenced using the `@{Section.key}` syntax.

```yini
[Display]
width = 1920

[Game]
render_width = @{Display.width}
```

### Environment Variable References
Environment variables can be referenced using the `${VAR_NAME}` syntax.

```yini
[System]
user_home = "${HOME}"
```