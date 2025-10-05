# YINI - A Modern INI for Game Development

YINI is a modern, feature-rich configuration file format based on the INI syntax, designed specifically for game development. It extends the traditional INI format with a variety of features, including section inheritance, dynamic values, and a rich set of data types, making it a powerful and flexible tool for managing complex game configurations.

YINI is built with C++17 for high performance and provides a seamless C# wrapper for use in .NET environments like Unity.

## Installation

### C# / .NET (Recommended)

The easiest way to use YINI in your .NET project is to install the official NuGet package.

```bash
dotnet add package Yini
```

This package includes the native C++ library and the C# wrapper, with all dependencies automatically handled.

### C++

For C++ projects, it is recommended to use a package manager like `vcpkg`.

```bash
vcpkg install yini
```

You can then link to the library in your `CMakeLists.txt` file:
```cmake
find_package(Yini REQUIRED)
target_link_libraries(your_target PRIVATE Yini::Yini)
```

## Features

*   **High-Performance:** The core library is written in modern C++ for maximum performance, with a source-generated C# wrapper that avoids reflection overhead.
*   **Rich Data Types:** Supports integers, floats, booleans, strings, arrays, maps, and more.
*   **Dynamic Values:** Define values that can be updated at runtime without re-parsing the entire file.
*   **Section Inheritance:** Create complex configurations by inheriting and overriding values from other sections.
*   **Macros and Variables:** Define and reuse values throughout your configuration files.
*   **File Includes:** Split your configuration into multiple, manageable files.

## High-Performance Binding with Source Generation

For C# projects, YINI provides a powerful source generator that creates high-performance, reflection-free binding code at compile time. To use it, simply annotate your class with the `[YiniBindable]` attribute.

**YINI File (`player.yini`):**
```yini
[playerstats]
name = Jules
level = 99
health = 125.5
is_active = true
```

**C# Code:**
```csharp
// Add this attribute to enable source generation
[YiniBindable]
public partial class PlayerStats
{
    // Use the YiniKey attribute to map to snake_case keys
    [YiniKey("name")]
    public string Name { get; set; }

    [YiniKey("level")]
    public int Level { get; set; }

    [YiniKey("health")]
    public double Health { get; set; }

    [YiniKey("is_active")]
    public bool IsActive { get; set; }
}

// ...

var manager = new YiniManager();
manager.Load("player.yini");

var stats = new PlayerStats();
// This method is generated at compile time and is extremely fast!
stats.BindFromYini(manager, "playerstats");

// The 'stats' object is now populated with the values from the file.
```

## Getting Started

*   **[YINI Language Manual](YINI.md):** A comprehensive guide to the YINI language and its features.
*   **[Building and Integrating YINI](docs/Building.md):** Instructions on how to build and integrate YINI from source.
*   **[C# API Reference](docs/CSharpAPI.md):** Detailed documentation for the YINI C# API.

## Contributing

We welcome contributions to the YINI project! If you're interested in contributing, please check out our [contribution guidelines](CONTRIBUTING.md).