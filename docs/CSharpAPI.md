# YINI C# API Reference

This document provides a reference for the YINI C# API, which is the recommended way to interact with the YINI library in .NET.

## YiniManager Class

The `YiniManager` class is the primary entry point for all interactions with the YINI library. It handles loading files, querying values, and saving changes. It implements `IDisposable` to manage the lifetime of the underlying native resources, so it should always be used within a `using` block or have its `Dispose()` method called explicitly.

### Constructor

**`YiniManager()`**

Creates a new instance of the `YiniManager` class.

### Key Methods

**`void Load(string filepath)`**

Loads and parses a YINI file from the specified path. This method will throw a `YiniException` if the file cannot be found, fails to parse, or if any other error occurs during the loading process.

**`void SaveChanges()`**

Saves any changes made to dynamic values back to the original file. This method performs a non-destructive write-back that preserves formatting and comments. It will throw a `YiniException` if it fails to write to the file.

**`bool HasKey(string section, string key)`**

Checks for the existence of a key within a given section. This is the most efficient way to check if a value is present without the overhead of retrieving the entire value.

*   **Returns:** `true` if the key exists, `false` otherwise.

**Example:**
```csharp
if (manager.HasKey("Player", "Name"))
{
    string name = manager.GetString("Player", "Name");
    Console.WriteLine($"Player Name: {name}");
}
```

**`YiniValue? GetValue(string section, string key)`**

Retrieves a value from the loaded YINI data as a `YiniValue` object. This is the most flexible way to get a value, as it allows you to inspect its type before converting it.

*   **Returns:** A `YiniValue` instance if the key is found; otherwise, `null`. The returned `YiniValue` is a disposable object and should be managed with a `using` block.

**`void SetValue(string section, string key, YiniValue value)`**

Sets the value for a specific key in a section. This is only valid for keys that were declared as dynamic (i.e., with `Dyna()`). Throws a `YiniException` on failure.

### Convenience Getters/Setters

The `YiniManager` provides several convenience methods for getting and setting primitive types. These methods are built on top of `GetValue` and `SetValue`.

*   **`GetDouble(string section, string key, double defaultValue = 0.0)`**
*   **`GetString(string section, string key, string defaultValue = "")`**
*   **`GetBool(string section, string key, bool defaultValue = false)`**
*   **`SetDouble(string section, string key, double value)`**
*   **`SetString(string section, string key, string value)`**
*   **`SetBool(string section, string key, bool value)`**

### Data Binding

**`T Bind<T>(string section) where T : new()`**

Binds a YINI section to a new instance of a C# class `T`. This method uses reflection to match the public properties of your class to the keys in the specified section. By convention, property names are converted to lowercase to find the corresponding key in the YINI file. For more advanced binding, you can use the `[YiniKey("custom-key")]` attribute on a property.

*   **`T`**: The type of the object to create and populate. Must have a parameterless constructor.
*   **`section`**: The name of the section in the YINI file to bind from.

**Example:**
```csharp
public class PlayerStats
{
    public string Name { get; set; }
    public int Level { get; set; }
    public double Health { get; set; }
}

var manager = new YiniManager();
manager.Load("stats.yini");

// Binds the [playerstats] section to the PlayerStats object.
PlayerStats stats = manager.Bind<PlayerStats>("playerstats");
```

## YiniException Class

The `YiniException` is thrown when an error occurs in the underlying native YINI library. This provides a much more robust error-handling mechanism than returning `bool` or `null`. You should wrap calls to `Load`, `SaveChanges`, and `SetValue` in a `try-catch` block to handle potential errors gracefully.

**Example:**
```csharp
var manager = new YiniManager();
try
{
    manager.Load("path/to/invalid/file.yini");
}
catch (YiniException ex)
{
    Console.WriteLine($"Failed to load YINI file: {ex.Message}");
}
```