# YINI Cookbook

This "Cookbook" provides practical, copy-and-paste examples for common tasks and problems you might encounter when using YINI in your projects.

---

## C#: Binding a YINI File to a C# Object

One of the most powerful features of the YINI C# wrapper is its ability to automatically bind a YINI section to a C# class. This is especially useful for loading settings or character stats.

### The YINI File (`PlayerStats.yini`)

```yini
[Stats]
Name = "Jules"
Level = 99
Health = 125.5
IsActive = true
PrimaryWeapon = "Sword of CSharp"
```

### The C# Code

First, define a C# class that matches the structure of your YINI section. You can use the `YiniKey` attribute if your C# property name doesn't match the YINI key name exactly.

```csharp
// PlayerStats.cs
using Yini;

public class PlayerStats
{
    // This property will be bound to the "Name" key in the [Stats] section
    public string Name { get; set; }

    // The rest of the properties are bound by matching the property name to the key name
    public int Level { get; set; }
    public double Health { get; set; }
    public bool IsActive { get; set; }

    [YiniKey("PrimaryWeapon")]
    public string MainHand { get; set; }
}
```

Next, use the `YiniManager.Bind<T>()` method to populate an instance of your class.

```csharp
// Program.cs
using Yini;
using System;

try
{
    using (var yini = new YiniManager())
    {
        yini.Load("PlayerStats.yini");

        // Bind the "[Stats]" section to a new PlayerStats object
        PlayerStats stats = yini.Bind<PlayerStats>("Stats");

        Console.WriteLine($"Player: {stats.Name}");
        Console.WriteLine($"Level: {stats.Level}");
        Console.WriteLine($"Weapon: {stats.MainHand}");
    }
}
catch (Exception ex)
{
    Console.WriteLine($"An error occurred: {ex.Message}");
}
```

---

## C++: Loading and Reading Configuration

This example demonstrates the fundamental C++ workflow: loading a file and retrieving values from it.

### The YINI File (`GameSettings.yini`)

```yini
[Graphics]
ResolutionX = 1920
ResolutionY = 1080
Fullscreen = true

[Audio]
MasterVolume = 0.8
MusicVolume = 0.6
```

### The C++ Code

```cpp
#include <iostream>
#include <Yini/YiniManager.h>
#include <Yini/YiniValue.h>

int main()
{
    try
    {
        YINI::YiniManager manager;
        if (!manager.load_from_file("GameSettings.yini"))
        {
            std::cerr << "Failed to load GameSettings.yini" << std::endl;
            return 1;
        }

        // Retrieve graphics settings
        auto resolutionX = manager.get_value("Graphics", "ResolutionX");
        auto fullscreen = manager.get_value("Graphics", "Fullscreen");

        if (resolutionX.has_value() && fullscreen.has_value())
        {
            // Use std::get to access the value in the variant
            double resX = std::get<double>(resolutionX->m_value);
            bool isFullscreen = std::get<bool>(fullscreen->m_value);

            std::cout << "Resolution: " << resX << std::endl;
            std::cout << "Fullscreen: " << (isFullscreen ? "Enabled" : "Disabled") << std::endl;
        }

        // Retrieve audio settings
        auto masterVolume = manager.get_double("Audio", "MasterVolume", 1.0);
        std::cout << "Master Volume: " << masterVolume << std::endl;
    }
    catch (const YINI::YiniException& e)
    {
        std::cerr << "A YINI error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```