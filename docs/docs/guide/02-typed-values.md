---
sidebar_position: 2
title: Typed Values
---

# Typed Values

Unlike traditional INI files that treat all values as strings, YINI has a rich type system that allows you to work with native data types directly in your configuration. This makes your configuration more robust and easier to integrate with your application.

## Primitive Types

### Strings

Strings are sequences of characters enclosed in double quotes.

```yini
message = "Hello, World!"
```

### Numbers

YINI supports both integers and floating-point numbers.

```yini
integer_value = 123
float_value = 3.14
```

### Booleans

Boolean values are represented by the keywords `true` and `false`.

```yini
is_enabled = true
is_hidden = false
```

## Collection Types

### Arrays

Arrays are ordered collections of values, enclosed in square brackets `[]`. They can contain mixed types.

```yini
simple_array = [1, 2, 3]
mixed_array = [1, "two", true]
nested_array = [[1, 2], [3, 4]]
```

You can also declare an array using a function-style syntax:

```yini
function_array = Array(1, 2, 3)
```

### Lists

Lists are similar to arrays but are created with the `List()` syntax. They are useful when you need to distinguish between a list and an array in your application logic.

```yini
player_scores = List(100, 95, 80)
```

### Sets

Sets are unordered collections of unique values, enclosed in parentheses `()`. Duplicate values are automatically removed.

To distinguish a single-element set from a grouped arithmetic expression, a trailing comma is required.

```yini
# The value "fast" will only be included once.
player_tags = ("fast", "player", "fast")

# A single-element set requires a trailing comma.
single_item_set = ("lonely",)

# An empty set is also valid.
empty_set = ()
```

### Maps

Maps (also known as dictionaries or associative arrays) are collections of key-value pairs, enclosed in curly braces `{}`.

```yini
player_stats = {
    level: 10,
    class: "Warrior",
    "home-town": "Capital City"
}
```

## Custom Data Types

YINI supports several special data types tailored for common application needs, like game development.

### Color

Colors can be defined in two ways:
-   **Hex Code**: `#RRGGBB` format.
-   **Function**: `Color(r, g, b)` with decimal values from 0-255.

```yini
health_bar_color = #FF0000
mana_bar_color = Color(0, 0, 255)
```

### Coord

Coordinates represent a point in 2D or 3D space and are created with the `Coord()` function.

```yini
spawn_point_2d = Coord(100.5, 50.0)
camera_position_3d = Coord(0, 10, -5)
```

### Path

Paths represent a file path and are created with the `Path()` function. They are parsed as a single string literal.

```yini
player_model = Path("assets/models/player.fbx")
```