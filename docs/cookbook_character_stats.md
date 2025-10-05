# Cookbook: Managing Character Stats

This guide demonstrates a practical, real-world example of using YINI to manage character statistics in a game. We'll cover defining base character classes, creating specific characters with unique stats, applying equipment bonuses, and accessing the data in C#.

## 1. Defining Base Character Templates

A powerful feature of YINI is section inheritance. We can use it to create templates for different character archetypes. Let's define a base template for all characters and a more specific one for melee fighters.

**`templates.yini`**
```yini
// templates.yini

[BaseCharacter]
health = 100
mana = 50
stamina = 100

[BaseMelee] : BaseCharacter
// Melee characters have higher strength and constitution by default
strength = 15
dexterity = 10
constitution = 15
intelligence = 8
wisdom = 8
charisma = 10
```

Here, `[BaseMelee]` inherits all the key-value pairs from `[BaseCharacter]` and adds its own.

## 2. Defining a Specific Character

Now, let's create a file for our player character, Grog the Barbarian. We'll use the `[#include]` feature to pull in our templates and `[#define]` to manage equipment bonuses.

**`grog_the_barbarian.yini`**
```yini
// grog_the_barbarian.yini

[#include]
+= "templates.yini"

[#define]
// --- Equipment Bonuses ---
great_axe_damage = 12
gauntlets_of_strength_bonus = 2
boots_of_speed_bonus = 5

// --- Grog's Base Stats (Overrides template) ---
base_strength = 18
base_constitution = 16

[Player_Grog] : BaseMelee
// Grog is a mighty barbarian, not a magic user.
name = "Grog"
mana = 0

// Use macros and arithmetic to calculate final stats
strength = @base_strength + @gauntlets_of_strength_bonus
constitution = @base_constitution
speed = 30 + @boots_of_speed_bonus

// Use Dyna() for values that change during gameplay
current_health = Dyna(@health)
is_enraged = Dyna(false)
```

In this file:
- We include `templates.yini` to get our base stats.
- We use a `[#define]` block to manage bonuses from Grog's equipment. This makes it easy to see and manage all modifiers in one place.
- The `[Player_Grog]` section inherits from `[BaseMelee]`.
- We override some stats like `mana` and define new calculated stats like `strength` and `speed`.
- We use `Dyna()` for `current_health` and `is_enraged`, as these values will likely change during gameplay.

## 3. Accessing Character Stats in C#

Finally, let's see how to load Grog's data in C# and bind it to a class. This demonstrates how easily you can integrate your YINI configuration into your game logic.

First, define a C# class to hold the stats. We'll use the `[YiniBindable]` attribute to enable high-performance, source-generated binding.

```csharp
// GrogStats.cs
using Yini;

[YiniBindable]
public partial class GrogStats
{
    // The YiniKey attribute maps a C# property to a YINI key.
    // If omitted, it defaults to the lower-cased property name.
    [YiniKey("name")]
    public string Name { get; set; }

    public double Strength { get; set; }
    public double Constitution { get; set; }
    public double Speed { get; set; }

    [YiniKey("current_health")]
    public double CurrentHealth { get; set; }

    [YiniKey("is_enraged")]
    public bool IsEnraged { get; set; }
}
```

Now, use `YiniManager` to load the file and bind the data.

```csharp
// GameManager.cs
using Yini;
using System;

public class GameManager
{
    public void LoadCharacter()
    {
        using var manager = new YiniManager();
        manager.Load("grog_the_barbarian.yini");

        var grog = new GrogStats();
        // The source generator creates this extension method for us!
        grog.BindFromYini(manager, "Player_Grog");

        Console.WriteLine($"Loaded character: {grog.Name}");
        Console.WriteLine($"Strength: {grog.Strength}"); // Outputs: 20
        Console.WriteLine($"Speed: {grog.Speed}");     // Outputs: 35

        // You can now modify dynamic values
        Console.WriteLine($"Is Grog enraged? {grog.IsEnraged}");
        manager.SetBool("Player_Grog", "is_enraged", true);
        Console.WriteLine($"Is Grog enraged now? {manager.GetBool("Player_Grog", "is_enraged")}");

        // Save the changes back to the file system
        manager.SaveChanges();
    }
}
```

This example shows a powerful and maintainable workflow for managing complex game data using YINI's features.