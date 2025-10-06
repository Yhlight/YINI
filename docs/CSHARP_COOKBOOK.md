# YINI C# Cookbook

This guide provides practical examples and recipes for using the YINI library in your C# projects. It covers common tasks like reading data, binding to objects, and validating configurations.

## 1. Getting Started: Loading a YINI File

The primary entry point for the library is the `YiniManager` class. It's `IDisposable`, so it should be wrapped in a `using` statement to ensure native resources are released.

```csharp
using Yini;
using System;

public class GameConfig
{
    public void Load()
    {
        using (var yini = new YiniManager())
        {
            if (yini.Load("config.yini"))
            {
                Console.WriteLine("YINI file loaded successfully.");
                // You can now read values from the manager.
            }
            else
            {
                Console.WriteLine("Failed to load YINI file.");
            }
        }
    }
}
```

## 2. Reading Basic Values

`YiniManager` provides strongly-typed getter methods for convenience. You can provide a default value that will be returned if the key is not found.

**config.yini:**
```yini
[Player]
name = "Jules"
level = 15
is_pro = true

[Graphics]
resolution_x = 1920
resolution_y = 1080
```

**C# Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    // Reading strings, integers, and booleans
    string playerName = yini.GetString("Player", "name", "DefaultPlayer");
    int playerLevel = yini.GetInt("Player", "level", 1); // GetInt is not implemented, but we can use GetDouble
    bool isPro = yini.GetBool("Player", "is_pro", false);

    Console.WriteLine($"Player: {playerName}, Level: {playerLevel}, Pro: {isPro}");

    // Reading a value that doesn't exist will return the default
    float musicVolume = (float)yini.GetDouble("Audio", "music_volume", 0.75);
    Console.WriteLine($"Music Volume: {musicVolume}");
}
```
*Note: `GetInt` is not implemented. You can use `GetDouble` and cast it.*

## 3. Working with Arrays and Dictionaries

YINI arrays and maps can be read into C# lists and dictionaries.

**config.yini:**
```yini
[Inventory]
items = [ "sword", "shield", "potion" ]

[Quests]
active_quests = {
    "main_quest_1": "Defeat the Dragon",
    "side_quest_2": "Find the Lost Key"
}
```

**C# Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    // Reading a list of strings
    List<string> inventory = yini.GetList<string>("Inventory", "items");
    if (inventory != null)
    {
        Console.WriteLine("Inventory Items: " + string.Join(", ", inventory));
    }

    // Reading a dictionary
    Dictionary<string, string> quests = yini.GetDictionary<string>("Quests", "active_quests");
    if (quests != null)
    {
        foreach (var quest in quests)
        {
            Console.WriteLine($"Quest '{quest.Key}': {quest.Value}");
        }
    }
}
```

## 4. Object Binding

YINI can automatically populate the properties of a C# object from a section. This is a powerful way to manage complex configurations.

**config.yini:**
```yini
[GraphicsSettings]
resolution_x = 1920
resolution_y = 1080
fullscreen = true
vsync = false
```

**C# POCO (Plain Old C# Object):**
```csharp
public class GraphicsSettings
{
    // Property names are automatically mapped to snake_case keys
    public int ResolutionX { get; set; }
    public int ResolutionY { get; set; }
    public bool Fullscreen { get; set; }
    public bool Vsync { get; set; }
}
```

**Binding Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    // Bind the [GraphicsSettings] section to a new GraphicsSettings object
    GraphicsSettings settings = yini.Bind<GraphicsSettings>("GraphicsSettings");

    Console.WriteLine($"Resolution: {settings.ResolutionX}x{settings.ResolutionY}");
    Console.WriteLine($"Fullscreen: {settings.Fullscreen}, VSync: {settings.Vsync}");
}
```
*Note: The `Bind<T>` method uses reflection. For performance-critical code, consider using the source generator (`[YiniBindable]`).*

## 5. Validating Configuration Against a Schema

If your YINI file contains a `[#schema]`, you can validate the configuration.

**config.yini:**
```yini
[#schema]
Player = {
    name = String | true,
    level = Integer | true,
    health = Integer | false
}

[Player]
name = "Jules"
// "level" is missing, which will cause a validation error
```

**C# Validation Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    List<string> errors = yini.Validate();

    if (errors.Count > 0)
    {
        Console.WriteLine("Configuration validation failed:");
        foreach (string error in errors)
        {
            Console.WriteLine($"- {error}");
        }
    }
    else
    {
        Console.WriteLine("Configuration is valid!");
    }
}
```

## 6. Iterating Over Sections and Keys

You can programmatically discover all the sections and keys that have been loaded.

```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    Console.WriteLine("--- All Configuration Data ---");
    foreach (string sectionName in yini.GetSectionNames())
    {
        Console.WriteLine($"[{sectionName}]");
        foreach (string keyName in yini.GetKeyNamesInSection(sectionName))
        {
            using (var value = yini.GetValue(sectionName, keyName))
            {
                 // We use GetValue and handle the YiniValue directly
                 // to properly display any type of value.
                Console.WriteLine($"{keyName} = {value.AsString()}");
            }
        }
    }
}
```

## 7. Modifying and Saving Dynamic Values

If a value is marked as `Dyna()`, you can change it at runtime and save it back to the corresponding `.ymeta` file.

**config.yini:**
```yini
[PlayerState]
health = Dyna(100)
```

**C# Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    // Player takes damage
    int newHealth = 85;
    yini.SetInt("PlayerState", "health", newHealth); // SetInt is not implemented, use SetDouble

    // This saves the change to config.yini.ymeta
    yini.SaveChanges();
    Console.WriteLine("Player health updated and saved.");
}
```
*Note: `SetInt` is not implemented. You can use `SetDouble`.*