# YINI Language Reference

This document provides a comprehensive reference for the YINI language.

## 1. Basic Syntax

### Sections

Configuration is organized into sections, denoted by square brackets `[]`.

```yini
[SectionName]
```

### Key-Value Pairs

Within a section, configuration is defined as key-value pairs.

```yini
key = value
```

### Comments

YINI supports two styles of comments:

- **Single-line:** `//`
- **Block:** `/* ... */`

```yini
// This is a single-line comment
key = value /* This is a block comment */
```

## 2. Data Types

YINI supports a rich set of data types.

- **String:** `"Hello, World!"`
- **Number:** `123`, `3.14`
- **Boolean:** `true`, `false`
- **Array:** `[1, "two", true]`
- **Set:** `(1, "two", true)` (A single-element set requires a trailing comma: `(1,)`)
- **Map:** `{"key1": 1, "key2": "two"}`

## 3. Advanced Features

### Macro Definitions (`[#define]`)

You can define global constants (macros) in a special `[#define]` section.

```yini
[#define]
player_speed = 10.5
default_name = "Player"
```

### Macro References (`@`)

Macros can be referenced anywhere an expression is expected using the `@` symbol.

```yini
[Player]
speed = @player_speed
name = @default_name
```

### Section Inheritance

Sections can inherit values from one or more parent sections. Values from later parents override earlier ones, and the section's own values override any inherited ones.

```yini
[Base]
health = 100
mana = 50

[Mage] : Base
mana = 150 // Overrides Base

[Warrior] : Base
strength = 20

[Paladin] : Mage, Warrior // Inherits from both
health = 120 // Overrides Base
```

### File Inclusion (`[#include]`)

You can include other YINI files to better organize your configuration. Files are merged in order, with later files overriding earlier ones.

```yini
[#include]
+= "base_settings.yini"
+= "player_stats.yini"
```

### Dynamic Values (`Dyna()`)

Values can be marked as dynamic, allowing them to be changed at runtime and saved back to the original file non-destructively (preserving comments and formatting).

```yini
[Settings]
volume = Dyna(100) // This can be changed by the application
music_on = true
```

### Arithmetic Expressions

YINI supports basic arithmetic operations (`+`, `-`, `*`, `/`, `%`) between numbers and macro references. Parentheses can be used to control the order of operations.

```yini
[Calculations]
result = (@some_macro * 2) + 10
```