# YINI C# API Reference

This document provides a reference for the YINI C# API.

## YiniManager Class

The `YiniManager` class is the main entry point for interacting with the YINI library.

### Constructor

**`YiniManager()`**

Creates a new instance of the `YiniManager` class.

### Methods

**`bool Load(string filepath)`**

Loads a YINI file from the specified path.

**`void SaveChanges()`**

Saves any changes made to the YINI file.

**`double GetDouble(string section, string key, double defaultValue = 0.0)`**

Gets a double value from the specified section and key.

**`string GetString(string section, string key, string defaultValue = "")`**

Gets a string value from the specified section and key.

**`bool GetBool(string section, string key, bool defaultValue = false)`**

Gets a boolean value from the specified section and key.

**`void SetDouble(string section, string key, double value)`**

Sets a double value for the specified section and key.

**`void SetString(string section, string key, string value)`**

Sets a string value for the specified section and key.

**`void SetBool(string section, string key, bool value)`**

Sets a boolean value for the specified section and key.

**`void Dispose()`**

Disposes of the `YiniManager` instance and releases any unmanaged resources.

**`T Bind<T>(string section) where T : new()`**

Binds a YINI section to a new instance of a C# class `T`. This method uses reflection to match the public properties of your class to the keys in the specified section. By convention, property names are converted to lowercase to find the corresponding key in the YINI file.

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