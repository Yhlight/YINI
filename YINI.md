# YINI Language Reference Manual

YINI is a modern, feature-rich configuration file format designed for game development. It extends the traditional INI syntax with advanced features like schema validation, type safety, dynamic values, and a powerful macro system. This document provides a comprehensive guide to the YINI language.

## 1. Basic Syntax

### 1.1. Comments

YINI supports C-style single-line and multi-line comments.

-   **Single-line comments:** Start with `//` and continue to the end of the line.
-   **Multi-line comments:** Are enclosed between `/*` and `*/`.

```yini
// This is a single-line comment.
[Graphics]
resolution_x = 1920 // Screen width in pixels

/*
  This is a multi-line comment.
  It can span several lines.
*/
fullscreen = true
```

### 1.2. Sections and Keys

YINI files are organized into sections, defined by a name in `[]`. Each section contains key-value pairs.

```yini
[PlayerStats]
name = "Jules"
level = 10
exp = 2550.75
```

## 2. Data Types

YINI supports a rich set of built-in data types:

| Type | Example | Description |
| :--- | :--- | :--- |
| **Integer** | `123`, `-45` | A 64-bit signed integer. |
| **Long** | `9007199254740991` | A 64-bit signed long integer, for larger values. |
| **Float** | `3.14`, `-0.01`, `1.2e-5` | A double-precision floating-point number. |
| **Boolean** | `true`, `false` | A boolean value. |
| **String** | `"Hello"`, `'World'` | Text enclosed in double or single quotes. |
| **Array** | `[ 1, "two", 3.0, true ]` | An ordered, mixed-type list. |
| **Map** | `{ "key1": "value1", "health": 100 }` | A collection of key-value pairs. |

## 3. Advanced Features

### 3.1. Section Inheritance

A section can inherit keys and values from one or more parent sections, promoting reusability.

```yini
[BaseCharacter]
health = 100
mana = 50

// Player inherits from BaseCharacter
[Player] : BaseCharacter
name = "Jules"
```
The `Player` section will now have `health`, `mana`, and `name`.

### 3.2. Value Registration (`+=`)

The `+=` operator provides a convenient syntax for adding items to a list-like registry under a section.

```yini
[UnlockedWeapons]
+= "Sword"
+= "Axe"
+= "Bow"
```
This creates a `UnlockedWeapons` key containing an array: `["Sword", "Axe", "Bow"]`.

### 3.3. Dynamic Values (`Dyna`)

A value can be marked as `Dyna(initial_value)` to indicate that it can be changed at runtime and saved back non-destructively.

```yini
[PlayerState]
health = Dyna(100)
ammo = Dyna(50)
```

Changes to these values are stored in a corresponding `.ymeta` file.

### 3.4. Arithmetic Operations

YINI supports standard arithmetic operations (`+`, `-`, `*`, `/`, `%`) and parentheses for precedence, which can be used with numeric values and macros.

```yini
[#define]
base_damage = 10
level_multiplier = 1.5

[Player]
attack_damage = @base_damage * (1 + @level_multiplier)
```

## 4. Referencing & Templating

YINI provides a powerful system for referencing other values, macros, and environment variables.

### 4.1. Macros (`[#define]`)

The `[#define]` section is used to declare constants or templates that can be reused throughout your files. Reference them with `@name`.

```yini
[#define]
player_name = "Jules"
default_volume = 0.8

[UI]
playerNameLabel = @player_name

[Audio]
music_volume = @default_volume
```

### 4.2. Cross-Section References

You can reference a value from another section using the `@{section.key}` syntax.

```yini
[Graphics]
fullscreen = true

[UI]
// The UI should know if the game is in fullscreen
is_fullscreen = @{Graphics.fullscreen}
```

### 4.3. Environment Variables

Reference system environment variables using `${VAR_NAME:default_value}`. A default can be provided if the variable is not set.

```yini
[Settings]
// Use the system's USER variable, or "guest" if it's not defined
user_name = "${USER:guest}"
```

## 5. File Organization

### 5.1. File Includes (`[#include]`)

Split your configuration across multiple files using the `[#include]` section. Paths are relative to the file containing the include directive.

```yini
[#include]
+= "graphics_settings.yini"
+= "audio_settings.yini"
```
Values from later files will override earlier ones if keys conflict.

### 5.2. YMETA Files

For each YINI file loaded, a `.ymeta` file may be generated in the same directory. This file serves two purposes:
1.  **Caching:** It stores a cached representation of the parsed YINI file, speeding up subsequent loads.
2.  **Dynamic State:** It stores the current values of any `Dyna()` variables that have been modified at runtime. This allows state to persist without overwriting the original, hand-authored `.yini` file.

It is safe to delete `.ymeta` files; they will be regenerated on the next run. They should typically be added to your version control system's ignore list (e.g., `.gitignore`).

## 6. Schema Validation (`[#schema]`)

You can define a schema to enforce the structure and types of your configuration. The `[#schema]` section specifies rules for other sections.

### 6.1. Defining a Schema

The schema defines the expected type for each key and whether it is required.

```yini
[#schema]
PlayerStats = {
    // key = type | required?
    name = String | true,
    level = Integer | true,
    exp = Float | false, // 'exp' is optional
    inventory = Array | false
}

Graphics = {
    resolution_x = Integer | true,
    resolution_y = Integer | true,
    fullscreen = Boolean | true,
    vsync = Dyna(Boolean) | false // Can be a dynamic boolean
}
```

### 6.2. Validation Rules

-   **Types:** `String`, `Integer`, `Long`, `Float`, `Boolean`, `Array`, `Map`.
-   **Dynamic Types:** You can specify a dynamic type with `Dyna(TypeName)`, e.g., `Dyna(Integer)`.
-   **Required:** `true` means the key must be present in the section. `false` means it is optional.

If a file fails schema validation, the `YiniManager` will throw an exception with detailed errors.

## 7. Command-Line Interface (CLI)

YINI provides a `yini-cli` tool for working with `.yini` files.

-   **Check Syntax:**
    ```bash
    yini-cli check your_file.yini
    ```

-   **Validate Against Schema:**
    ```bash
    yini-cli validate your_file.yini
    ```

-   **Compile (Resolves and flattens a file):**
    ```bash
    yini-cli compile your_file.yini --output flat_file.yini
    ```

-   **Decompile (Formats a file):**
    ```bash
    yini-cli decompile your_file.yini --output formatted_file.yini
    ```