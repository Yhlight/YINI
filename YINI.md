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

## 6. Dynamic Values and Non-Destructive Saving

YINI supports dynamic values, which can be updated in real-time during gameplay. Dynamic values are created using the `Dyna()` function.

```yini
[Player]
// This value can be changed at runtime.
health = Dyna(100)
```

When a dynamic value is modified and saved, YINI performs a **non-destructive write-back**. It carefully updates the original `.yini` file, preserving all comments and existing formatting. This makes it safe to use dynamic values in configuration files that are also managed by humans.

## 7. Macros and Variables

YINI allows you to define macros in a `[#define]` section and reference them using the `@` symbol.

```yini
[#define]
player_name = "Jules"

[UI]
playerNameLabel = @player_name
```

## 8. File Includes

YINI supports file includes, which allow you to split your configuration into multiple files.

```yini
[#include]
+= "player.yini"
+= "monsters.yini"
```

The included files are merged in the order they are specified, with later files overriding earlier ones.

## 9. Arithmetic Operations

YINI supports basic arithmetic operations, including `+`, `-`, `*`, `/`, and `%`.

```yini
[#define]
base_damage = 10

[Player]
attack_damage = @base_damage * 1.5
```

## 11. CLI

YINI includes a command-line interface (CLI) tool for compiling, decompiling, and validating YINI files.