---
sidebar_position: 3
title: Advanced Features
---

# Advanced Features

YINI includes several advanced features that make it a powerful tool for managing complex configurations.

## Section Inheritance

You can create a new section that inherits all the key-value pairs from one or more parent sections. This is a powerful way to reduce duplication and create variations of a base configuration.

-   **Syntax**: `[ChildSection : Parent1, Parent2, ...]`
-   **Resolution**: If multiple parent sections define the same key, the value from the rightmost parent in the list takes precedence. Keys defined directly in the child section will always override any inherited values.

```yini
[BaseEnemy]
health = 100
speed = 10

// Goblin inherits from BaseEnemy
[Goblin : BaseEnemy]
speed = 12 // Overrides the inherited speed
weapon = "Club"

// ArmoredGoblin inherits from Goblin, which already inherited from BaseEnemy
[ArmoredGoblin : Goblin]
health = 150 // Overrides Goblin's (and BaseEnemy's) health
```

## Macros and Variables

YINI allows you to define macros (constants) that can be reused throughout your configuration files.

-   **Definition**: Macros are defined in a special `[#define]` section.
-   **Reference**: You can reference a macro using the `@` symbol followed by the macro name.

```yini
[#define]
base_damage = 10
version = "1.2.0"

[Player]
damage = @base_damage * 1.5 // Arithmetic is supported
game_version = @version
```

## File Includes

You can split your configuration across multiple files and merge them together using the `[#include]` directive. This is useful for organizing large and complex configurations.

-   **Syntax**: Use `+=` within an `[#include]` section to add another file.
-   **Resolution**: Files are merged in the order they are listed. If the same section or key exists in multiple files, the content from the later file will overwrite the content from the earlier one.

```yini
// in main.yini
[#include]
+= settings.yini
+= player_stats.yini

[Core]
main_setting = true // This will be merged with settings from the included files
```

## Dynamic Values (`Dyna()`)

The `Dyna()` function marks a value as "dynamic," meaning that changes made to it at runtime can be written back to the source `.yini` file. This is perfect for persisting user settings, such as volume controls or graphics options.

-   **Syntax**: Wrap a value with `Dyna()`.
-   **Behavior**: When the `YiniManager` is used to manage the file, any changes made to a `Dyna()` value will be saved back to the original `.yini` file when the manager is destroyed. This process is non-destructive and preserves comments and formatting.

```yini
[Settings]
// This value can be changed by the game and will be saved
master_volume = Dyna(100)

// This value is static and will not be written back
music_volume = 80
```