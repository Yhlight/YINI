# YINI Usage

This document describes how to use the YINI language and the accompanying tools.

## Language Features

YINI is an enhanced version of the INI format, designed for game development. It supports features like section inheritance, auto-indexed arrays, typed values, macros, and file includes.

- **Inheritance**: `[Child] : Parent1, Parent2`
- **Quick Registration**: `+= value`
- **Value Types**: String, Integer, Float, Boolean, Array `[...]`, Object `{...}`, Coord, Color.
- **Macros**: Define in `[#define]` and reference with `@name`.
- **Includes**: Load other files with `[#include]`.

---

## C++ Library Usage (Internal)

The core of YINI is a C++ library that can be integrated into C++ applications. The main entry point is the `Yini::Loader` class, which handles loading, parsing, and processing of YINI files.

---

## Command-Line Interface (CLI)

The `yini-cli` executable provides tools for managing YINI files.

- `compile <file.yini>`: Compiles a `.yini` file into a binary `.ymeta` cache.
- `decompile <file.ymeta>`: Reads a `.ymeta` file and prints its contents.
- `check <file.yini>`: Checks the syntax of a `.yini` file and its dependencies.
- `help`: Displays a list of commands.
- `exit`: Quits the CLI.

---

## Using YINI with C#

The YINI library can be used directly from C# via P/Invoke. A convenient C# wrapper is provided to make this easy.

### 1. Loading a YINI File

To load a YINI file, create an instance of the `Yini.YiniHandle` class within a `using` block. The `using` block ensures that the native memory is automatically freed when you are done.

```csharp
using Yini;

try
{
    using (var yini = new YiniHandle("path/to/your/config.yini"))
    {
        // ... read values here ...
    }
}
catch (Exception ex)
{
    Console.WriteLine($"Error loading YINI file: {ex.Message}");
}
```

### 2. Accessing Values

There are two ways to access values: an easy-to-use method based on JSON, and a high-performance granular API.

#### Method A: Easy Access with JSON (`GetValueAs<T>`)

This is the simplest way to get any value, especially complex ones. The C++ library serializes the value to a JSON string, and the C# wrapper deserializes it into the C# type you specify.

**Example:**
```csharp
// Define a C# class to match your object structure
public record MyEntity(string Id, int Level);

// Get a simple value
string title = yini.GetValueAs<string>("Window", "title");

// Get a list of custom objects
var entities = yini.GetValueAs<List<MyEntity>>("Entities", "definitions");
foreach (var entity in entities)
{
    Console.WriteLine($"Entity: {entity.Id}, Level: {entity.Level}");
}
```

#### Method B: High-Performance Granular API

For maximum performance, you can get handles to values and collections directly and iterate over them without JSON serialization. This API is more complex but avoids the overhead of string parsing.

**Example:**
```csharp
// Get a handle to the value, which we know is an array
Yini.YiniValue arrayValue = yini.GetValue("Entities", "definitions");

if (arrayValue != null && arrayValue.Type == Yini.YiniValueType.Array)
{
    // Get a handle to the array itself
    Yini.YiniArray entityArray = arrayValue.AsArray();

    Console.WriteLine($"Found {entityArray.Count} entities.");

    // The YiniArray wrapper implements IEnumerable, so you can use foreach
    foreach (Yini.YiniValue entityValue in entityArray)
    {
        // Each item in the array is another value handle (an object)
        if (entityValue.Type == Yini.YiniValueType.Object)
        {
            // We can now get the properties of the object using the JSON method
            string id = entityValue.GetValueAs<string>("id");
            Console.WriteLine($"  - Entity ID: {id}");
        }
    }
}
```
