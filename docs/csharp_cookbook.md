# YINI C# Cookbook

Welcome to the YINI C# Cookbook! This guide provides practical examples and recipes to help you use the Yini .NET library effectively in your projects.

## 1. Getting Started: Loading a YINI File

The primary entry point for interacting with YINI is the `YiniManager` class. To get started, create an instance of the manager and load your YINI file.

**Example `settings.yini`:**
```yini
[Graphics]
resolution_width = 1920
resolution_height = 1080
fullscreen = true

[Audio]
master_volume = 0.8
```

**C# Code:**
```csharp
using Yini;
using System;

public class GameSettingsLoader
{
    public void LoadSettings()
    {
        using (var manager = new YiniManager())
        {
            if (manager.Load("path/to/settings.yini"))
            {
                Console.WriteLine("Settings loaded successfully!");
            }
            else
            {
                Console.WriteLine("Failed to load settings.");
            }
        }
    }
}
```

## 2. Reading Basic Values

`YiniManager` provides simple methods to retrieve primitive data types. These methods include a `defaultValue` parameter, which is returned if the key is not found.

```csharp
using (var manager = new YiniManager())
{
    manager.Load("settings.yini");

    // Read integer (retrieved as double and cast)
    int width = (int)manager.GetDouble("Graphics", "resolution_width", 1280);

    // Read boolean
    bool isFullscreen = manager.GetBool("Graphics", "fullscreen", false);

    // Read float (retrieved as double and cast)
    float volume = (float)manager.GetDouble("Audio", "master_volume", 1.0);

    // Read a string (key doesn't exist, so default is returned)
    string playerName = manager.GetString("Player", "name", "Guest");

    Console.WriteLine($"Resolution: {width}x{1080}"); // 1080 is hardcoded for brevity
    Console.WriteLine($"Fullscreen: {isFullscreen}");
    Console.WriteLine($"Volume: {volume}");
    Console.WriteLine($"Player: {playerName}");
}
```

## 3. Working with Collections

YINI makes it easy to work with lists and dictionaries.

**Example `inventory.yini`:**
```yini
[PlayerInventory]
items = ["Sword", "Shield", "Health Potion"]
skill_levels = { "swords": 10, "archery": 5, "magic": 2 }
```

**C# Code:**
```csharp
using (var manager = new YiniManager())
{
    manager.Load("inventory.yini");

    // Read a list of strings
    List<string> items = manager.GetList<string>("PlayerInventory", "items");
    if (items != null)
    {
        Console.WriteLine("Inventory Items: " + string.Join(", ", items));
    }

    // Read a dictionary
    Dictionary<string, double> skills = manager.GetDictionary<double>("PlayerInventory", "skill_levels");
    if (skills != null)
    {
        foreach (var skill in skills)
        {
            Console.WriteLine($"Skill: {skill.Key}, Level: {skill.Value}");
        }
    }
}
```

## 4. High-Performance Binding with Source Generators

For performance-critical applications, YINI provides a source generator that creates reflection-free binding code at compile time. This is the recommended approach for loading complex objects.

To use it, mark your C# class with the `[YiniBindable]` attribute.

**Example `player_stats.yini`:**
```yini
[Stats]
name = "Jules"
level = 15
is_active = true
hit_points = 95.5
```

**C# Class Definition:**
```csharp
using Yini;

// This attribute tells the source generator to create a binder for this class.
[YiniBindable]
public partial class PlayerStats
{
    // The [YiniKey] attribute can be used to map a property to a different key name.
    // If omitted, the property name (converted to lower_case) is used as the key.
    [YiniKey("name")]
    public string PlayerName { get; set; } = "Default";

    public int Level { get; set; } = 1;

    [YiniKey("is_active")]
    public bool IsActive { get; set; }

    // This property will not be bound because it's missing a key in the YINI file.
    // Its default value will be preserved.
    public float Mana { get; set; } = 100.0f;

    // Properties are automatically mapped to keys, e.g., HitPoints -> hit_points
    public double HitPoints { get; set; }
}
```

**C# Usage:**
```csharp
using (var manager = new YiniManager())
{
    manager.Load("player_stats.yini");

    var stats = new PlayerStats();

    // Call the source-generated `BindFromYini` method for high-performance binding.
    stats.BindFromYini(manager, "Stats");

    Console.WriteLine($"Name: {stats.PlayerName}");       // Output: Jules
    Console.WriteLine($"Level: {stats.Level}");          // Output: 15
    Console.WriteLine($"Is Active: {stats.IsActive}");   // Output: True
    Console.WriteLine($"Mana: {stats.Mana}");            // Output: 100.0 (preserved default)
    Console.WriteLine($"Hit Points: {stats.HitPoints}"); // Output: 95.5
}
```
This approach avoids the overhead of reflection, making it ideal for loading configuration data at startup or during gameplay.