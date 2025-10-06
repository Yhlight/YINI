using System;
using System.IO;
using System.Collections.Generic;
using Yini;

// This partial class is annotated with [YiniBindable].
// At compile time, a high-performance `BindFromYini` method will be
// generated for it. PascalCase properties are automatically mapped
// to snake_case keys in the YINI file (e.g., Health -> health).
[YiniBindable]
public partial class Character
{
    public string Name { get; set; } = "Unknown";
    public int Level { get; set; }
    public double Health { get; set; }
    public double Mana { get; set; }
    public int Strength { get; set; }
    public int Intelligence { get; set; }
    public List<string> Inventory { get; set; } = new();
    public Dictionary<string, int> Skills { get; set; } = new();
}

public class Program
{
    public static void Main()
    {
        string filePath = "stats.yini";
        if (!File.Exists(filePath))
        {
            Console.WriteLine($"Error: '{filePath}' not found. Make sure you are running this from the 'examples/character_stats' directory.");
            return;
        }

        using var manager = new YiniManager();

        Console.WriteLine($"Loading character stats from '{filePath}'...");
        manager.Load(filePath);

        // 1. Create a character and bind its stats from the "Grok" section.
        //    The BindFromYini method is generated at compile time and is very fast.
        //    Notice that all values from [BaseCharacter] and [Warrior] are
        //    correctly inherited.
        Console.WriteLine("\n--- Binding Character 'Grok' ---");
        var grok = new Character();
        grok.BindFromYini(manager, "Grok");

        // 2. Print the character's stats to verify the binding.
        Console.WriteLine($"Name: {grok.Name}");
        Console.WriteLine($"Level: {grok.Level}");
        Console.WriteLine($"Health: {grok.Health}, Mana: {grok.Mana}");
        Console.WriteLine($"Strength: {grok.Strength}, Intelligence: {grok.Intelligence}");

        Console.WriteLine("\nInventory:");
        foreach (var item in grok.Inventory)
        {
            Console.WriteLine($"- {item}");
        }

        Console.WriteLine("\nSkills:");
        foreach (var skill in grok.Skills)
        {
            Console.WriteLine($"- {skill.Key}: Level {skill.Value}");
        }
    }
}