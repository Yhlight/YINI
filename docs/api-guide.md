# YINI C# API Guide

This guide explains how to use the C# `YiniManager` wrapper to interact with the YINI library from a .NET application.

## 1. Getting Started

First, add a reference to the `Yini.dll` in your C# project. The `YiniManager` class is the main entry point for all operations.

## 2. Loading a YINI File

To load a configuration file, create an instance of `YiniManager` and call the `Load` method. It's recommended to wrap the manager in a `using` statement to ensure that unmanaged resources are correctly disposed of.

```csharp
using Yini;

try
{
    using (var yini = new YiniManager())
    {
        bool success = yini.Load("path/to/your/config.yini");
        if (success)
        {
            Console.WriteLine("YINI file loaded successfully.");
        }
        else
        {
            Console.WriteLine("Failed to load YINI file.");
        }
    }
}
catch (Exception ex)
{
    Console.WriteLine($"An error occurred: {ex.Message}");
}
```

## 3. Getting Values

You can retrieve values from the loaded configuration using the `Get...` methods.

- `GetDouble(string section, string key, double defaultValue = 0.0)`
- `GetString(string section, string key, string defaultValue = "")`
- `GetBool(string section, string key, bool defaultValue = false)`

```csharp
// Assuming [Player] section with key 'speed = 10.5'
double speed = yini.GetDouble("Player", "speed");

// Assuming [Settings] section with key 'username = "Jules"'
string username = yini.GetString("Settings", "username");
```

## 4. Setting Dynamic Values

You can only modify values that were declared with `Dyna()` in the `.yini` file.

- `SetDouble(string section, string key, double value)`
- `SetString(string section, string key, string value)`
- `SetBool(string section, string key, bool value)`

```csharp
// Assuming [Settings] volume = Dyna(100)
yini.SetDouble("Settings", "volume", 75.5);
```

## 5. Saving Changes

To persist any changes made to dynamic values, call the `SaveChanges` method. This will update the original `.yini` file non-destructively.

```csharp
yini.SaveChanges();
```