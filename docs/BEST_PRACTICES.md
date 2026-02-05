# YINI Best Practices

To get the most out of YINI in your game development workflow, consider the following recommendations.

## 1. Project Structure
Organize your configuration files logically, separating schemas from data.

```
/Config
  /Schemas          # Definitions of structure and types
    config.schema.yini
    items.schema.yini
  /Data             # Actual game data
    /Levels
      level1.yini
    game.yini
  /Include          # Shared macros or constants
    colors.yini
    macros.yini
```

## 2. Use Schemas Aggressively
Always define a `[#schema]` for your main configuration sections. This provides:
- **Validation:** Catch missing keys or wrong types at build time.
- **Documentation:** LSP shows types and comments to anyone editing the file.
- **Safety:** Prevents runtime crashes due to typos.

Example:
```ini
[#schema]
[Player]
Health = !, int, min=0
Speed = ?, float, =5.0
```

## 3. Leverage Includes for Composition
Don't put everything in one file. Use `[#include]` to compose larger configs from smaller, manageable pieces.

```ini
[#include]
+= "Includes/colors.yini"
+= "Data/Levels/level1.yini"
```

## 4. Differentiate Structs vs Maps
- Use **Structs** `{k:v}` for fixed data structures (like a coordinate or a specific object definition) where the keys are known.
- Use **Maps** `{k:v,}` for dynamic collections where keys might be arbitrary IDs (like an inventory or localized strings).

## 5. Dyna for Formulas, Not Logic
Use `Dyna()` for mathematical relationships (e.g., `Damage = Base * LevelMultiplier`), but avoid complex game logic. Keep logic in C# and configuration in YINI.

## 6. Binary for Release, Text for Dev
Use the `Yini.CLI` to compile `.ybin` files for your shipping builds to ensure fast loading and smaller size. Use `.yini` text files during development for easy version control and merging.
