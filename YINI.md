# YINI Language Manual

YINI is a modern configuration file format designed for performance-critical applications like game development. It extends the familiar INI syntax with a powerful set of features, including a rich type system, section inheritance, macros, schema validation, and seamless C# integration.

## 1. Core Concepts

### 1.1. Comments

YINI supports two comment styles:

-   **Single-line comments:** Start with `//` and continue to the end of the line.
-   **Multi-line comments:** Enclosed between `/*` and `*/`.

```yini
// This is a single-line comment.

/*
  This is a
  multi-line comment.
*/
```

### 1.2. Sections and Keys

Configuration is organized into sections, defined by a name in `[]`. Each section contains key-value pairs.

```yini
[Player]
name = "Jules"
level = 10
```

## 2. Data Types

YINI supports a rich set of data types.

| Type      | Example                                          | Description                                             |
| :-------- | :----------------------------------------------- | :------------------------------------------------------ |
| **Integer** | `42`                                             | A whole number.                                         |
| **Float**   | `3.14`                                           | A floating-point number.                                |
| **Boolean** | `true` or `false`                                | A boolean value.                                        |
| **String**  | `"Hello, World!"` or `'Hello, World!'`           | Text enclosed in single or double quotes.               |
| **Array**   | `[1, "two", 3.0, true]`                          | An ordered, mixed-type list.                            |
| **Set**     | `(1, "two", 3.0, true)`                          | An ordered, mixed-type list, similar to an array.       |
| **Map**     | `{ "key1": "value1", "key2": 123 }`              | A collection of key-value pairs. Keys must be strings.  |
| **Color**   | `Color(255, 192, 203)` or `Color(255,192,203,128)` | Represents an RGB or RGBA color.                        |
| **Vector**  | `Vec2(x, y)`, `Vec3(x, y, z)`, `Vec4(x, y, z, w)` | Represents 2D, 3D, or 4D coordinates.                   |
| **Path**    | `Path("/path/to/file")`                          | Represents a file system path.                          |
| **Dynamic** | `Dyna(initial_value)`                            | A value that can be changed at runtime and saved back.  |

## 3. Language Features

### 3.1. Value Registration

The `+=` operator provides a convenient way to build up a list of values under a single key, creating an array.

```yini
[Weapons]
available += "Sword"
available += "Axe"
available += "Bow"

// The 'available' key is now equivalent to:
// available = ["Sword", "Axe", "Bow"]
```

### 3.2. Section Inheritance

A section can inherit keys and values from one or more parent sections. Values from later parents or the child section itself will override earlier ones.

```yini
[BaseCharacter]
health = 100
mana = 50

[Warrior] : BaseCharacter
mana = 20 // Overrides mana from BaseCharacter
strength = 15

// The 'Warrior' section now effectively contains:
// health = 100, mana = 20, strength = 15
```

### 3.3. Macros (`[#define]`)

The `[#define]` section lets you declare constants that can be reused throughout your files. Reference a macro using the `@` symbol.

```yini
[#define]
primary_color = Color(255, 0, 0)
base_damage = 10

[UI]
button_color = @primary_color

[Player]
attack = @base_damage * 1.5
```

### 3.4. File Includes (`[#include]`)

Split your configuration across multiple files using the `[#include]` section. Paths are relative to the current file.

```yini
[#include]
+= "settings.yini"
+= "player_stats.yini"
```

### 3.5. Arithmetic Operations

YINI supports basic arithmetic (`+`, `-`, `*`, `/`, `%`). Use parentheses `()` for grouping and to control the order of operations.

```yini
[Calculations]
value = (10 + 5) * 2 // result is 30
```

### 3.6. Advanced References

#### Cross-Section References
Reference a value from another section using the `@{section.key}` syntax.

```yini
[Settings]
master_volume = 0.8

[Audio]
// References master_volume from the Settings section
music_volume = @{Settings.master_volume} * 0.5
```

#### Environment Variables
Pull values from your system's environment variables using the `${VAR_NAME}` syntax. You can also provide a default value with `${VAR_NAME:default}`.

```yini
[Database]
// Uses the value of the "DB_HOST" env var, or "localhost" if it's not set.
host = ${DB_HOST:localhost}
port = ${DB_PORT:5432}
```

## 4. Dynamic Values & Non-Destructive Saving

Values that need to be changed at runtime (e.g., player health, settings) can be marked as dynamic using `Dyna()`.

```yini
[Player]
// This value can be updated in code and saved.
health = Dyna(100)
```

When you save changes, YINI intelligently updates only the modified values in the original file, preserving all comments and formatting. This makes it safe to manage configuration files both by hand and programmatically.

## 5. Schema and Validation (`[#schema]`)

YINI allows you to embed a schema directly within your configuration files to enforce a specific structure. This is an optional but powerful feature for ensuring configuration correctness.

The `[#schema]` block defines the expected sections and keys, along with their types and whether they are required.

```yini
[#schema]
[Player]
name: string!      // required string
class: string      // optional string
level: integer!    // required integer
inventory: [item]  // optional array of 'item' type (defined below)

[item]
id: string!
quantity: integer
```

You can validate a file against its schema using the command-line tool.

## 6. Command-Line Interface (CLI)

YINI includes a CLI tool (`yini-cli`) for common operations.

| Command                       | Description                                                     |
| :---------------------------- | :-------------------------------------------------------------- |
| `check <filepath>`            | Checks the syntax of a `.yini` file for errors.                 |
| `validate <filepath>`         | Validates the file against its embedded `[#schema]`.            |
| `compile <in.yini> <out.ymeta>` | Compiles a `.yini` file into a binary `.ymeta` format.          |
| `decompile <in.ymeta>`        | Decompiles a `.ymeta` file and prints its contents to the console. |

## 7. C# / .NET Integration

YINI provides a high-performance C# library for seamless integration into .NET projects.

### 7.1. Basic Usage

The `YiniManager` class is the main entry point.

```csharp
using var manager = new YiniManager();
manager.Load("config.yini");

// Retrieve values
string name = manager.GetString("Player", "name", "Default");
double health = manager.GetDouble("Player", "health", 100.0);
bool active = manager.GetBool("Player", "is_active", false);

// Set a dynamic value and save
manager.SetDouble("Player", "health", 85.5);
manager.SaveChanges();
```

### 7.2. Complex Types

Convenience methods exist for retrieving collections and special types.

```csharp
// Get a list of strings
List<string> items = manager.GetList<string>("Inventory", "items");

// Get a dictionary
Dictionary<string, int> ammo = manager.GetDictionary<int>("Inventory", "ammo");

// Get special vector types (requires System.Numerics)
Vector3 position = manager.Get<Vector3>("Player", "position");

// Get a color type
Color color = manager.Get<Color>("UI", "theme_color");
```

### 7.3. Data Binding

YINI offers two ways to bind configuration data directly to C# objects.

#### Reflection-Based Binding
The `Bind<T>` method provides an easy way to populate an object's properties. Use the `[YiniKey]` attribute if a property name doesn't match the snake_case key name.

```csharp
public class GraphicsSettings
{
    [YiniKey("resolution_x")]
    public int Width { get; set; }

    [YiniKey("resolution_y")]
    public int Height { get; set; }

    public bool VSync { get; set; } // Auto-maps to "v_sync"
}

// ...
GraphicsSettings settings = manager.Bind<GraphicsSettings>("Graphics");
```

#### High-Performance Source-Generated Binding
For maximum performance with zero reflection, annotate your class with `[YiniBindable]`. This generates a `BindFromYini` method at compile time.

- `PascalCase` properties are automatically converted to `snake_case` keys.
- The `[YiniKey]` attribute can be used to override the default name.

```csharp
[YiniBindable]
public partial class PlayerStats
{
    public string PlayerName { get; set; } // Maps to "player_name"

    [YiniKey("character_class")]
    public string Class { get; set; }
}

// ...
var stats = new PlayerStats();
// This generated method is extremely fast!
stats.BindFromYini(manager, "Player");
```