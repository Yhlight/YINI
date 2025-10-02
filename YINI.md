# YINI Language Manual

YINI is a modern configuration file format designed for game development. It extends the traditional INI syntax with a variety of features that make it more powerful and flexible. This document provides a comprehensive guide to the YINI language and its features.

## 1. Comments

YINI supports both single-line and multi-line comments.

-   **Single-line comments:** Start with `//` and continue to the end of the line.
-   **Multi-line comments:** Start with `/*` and end with `*/`.

```yini
// This is a single-line comment.

/*
  This is a
  multi-line comment.
*/
```

## 2. Sections and Keys

YINI files are organized into sections, which are defined by a name enclosed in square brackets (`[]`). Each section contains a set of key-value pairs.

```yini
[Player]
name = "Jules"
level = 10
```

## 3. Data Types

YINI supports a variety of data types, including:

-   **Integer:** `123`
-   **Float:** `3.14`
-   **Boolean:** `true` or `false`
-   **String:** `"Hello, World!"`
-   **Array:** `[1, 2, 3]`
-   **Set:** `(1, 2, 3)`
-   **Map:** `{ "key1": "value1", "key2": "value2" }`
-   **Color:** `#FFC0CB` or `Color(255, 192, 203)`
-   **Coordinate:** `Coord(1, 2, 3)`
-   **Path:** `Path("/path/to/file")`
-   **List:** `List(1, 2, 3)`

## 4. Inheritance

YINI supports section inheritance, which allows you to create a new section that inherits the key-value pairs from one or more parent sections.

```yini
[BaseCharacter]
health = 100
mana = 50

[Player] : BaseCharacter
name = "Jules"
```

In this example, the `Player` section inherits the `health` and `mana` keys from the `BaseCharacter` section.

## 5. Value Registration

YINI provides a shortcut for registering values in a list-like fashion using the `+=` operator. This is useful for creating registries of items, such as a list of available weapons in a game.

```yini
[Weapons]
+= "Sword"
+= "Axe"
+= "Bow"
```

## 6. Dynamic Values

YINI supports dynamic values, which can be updated in real-time during gameplay. Dynamic values are created using the `Dyna()` function.

```yini
[Player]
health = Dyna(100)
```

When a dynamic value is modified, the change is written to a `.ymeta` file, which is a metadata file that stores the dynamic state of the YINI file.

## 7. Macros and Variables

YINI allows you to define macros in a `[#define]` section and reference them using the `@` symbol.

```yini
[#define]
player_name = "Jules"

[UI]
playerNameLabel = @player_name
```

## 8. Environment Variable Substitution

YINI supports substituting values from system environment variables using the `${VAR_NAME}` syntax. This is particularly useful for providing sensitive data like API keys or passwords without hardcoding them into your configuration files.

If the environment variable is set, its value will be used. If it is not set, it will be replaced with an empty string.

```yini
[Database]
# The password will be read from the DB_PASSWORD environment variable
password = ${DB_PASSWORD}

[API]
# The API key will be read from the API_KEY environment variable
key = ${API_KEY}
```

## 9. File Includes

YINI supports file includes, which allow you to split your configuration into multiple files.

```yini
[#include]
+= "player.yini"
+= "monsters.yini"
```

The included files are merged in the order they are specified, with later files overriding earlier ones.

## 10. Arithmetic Operations

YINI supports basic arithmetic operations, including `+`, `-`, `*`, `/`, and `%`.

```yini
[#define]
base_damage = 10

[Player]
attack_damage = @base_damage * 1.5
```

## 11. YMETA Files

For each YINI file, a corresponding `.ymeta` file is generated to cache information and store dynamic values. This improves performance by avoiding the need to re-parse the YINI file on every load.

## 12. CLI

YINI includes a command-line interface (CLI) tool for compiling, decompiling, and validating YINI files.