using System;
using System.Collections.Generic;
using System.IO;
using Yini;

// Define some C# types to deserialize into
public record Entity(string Id, string Texture);

// --- 1. Create a test YINI file with complex types ---
const string yiniFile = "test.yini";
File.WriteAllText(yiniFile, @"
[Display]
title = ""YINI C# Example""

[Entities]
definitions = [
    { id: ""player"", texture: ""player.png"" },
    { id: ""enemy"",  texture: ""enemy.png"" }
]
");

Console.WriteLine("--- C# YINI P/Invoke Example (Advanced) ---");
Console.WriteLine($"Created temporary config file: '{yiniFile}'");

try
{
    using (var yini = new YiniHandle(yiniFile))
    {
        Console.WriteLine("\nSuccessfully loaded YINI file.");

        // --- Method 1: Easy access with JSON ---
        Console.WriteLine("\n--- 1. Reading via JSON (GetValueAs<T>) ---");
        var entities = yini.GetValueAs<List<Entity>>("Entities", "definitions");
        Console.WriteLine("Got entity list via JSON deserialization:");
        foreach (var entity in entities)
        {
            Console.WriteLine($"  - Entity ID: {entity.Id}, Texture: {entity.Texture}");
        }

        // --- Method 2: High-performance granular API ---
        Console.WriteLine("\n--- 2. Reading via Granular API ---");
        YiniValue listValue = yini.GetValue("Entities", "definitions");

        if (listValue != null && listValue.Type == YiniValueType.Array)
        {
            YiniArray yiniArray = listValue.AsArray();
            Console.WriteLine($"Iterating through array of size {yiniArray.Count} using granular API:");

            foreach (YiniValue itemValue in yiniArray)
            {
                // Each item in the array is an object.
                // We can now deserialize it directly from the value handle.
                var entity = itemValue.As<Entity>();
                Console.WriteLine($"  - Entity ID: {entity.Id}, Texture: {entity.Texture}");
            }
        }
        else
        {
            Console.WriteLine("Could not get 'definitions' as an array via granular API.");
        }
    }
}
catch (Exception ex)
{
    Console.ForegroundColor = ConsoleColor.Red;
    Console.WriteLine($"\nAn error occurred: {ex.Message}");
    Console.ResetColor();
}
finally
{
    if (File.Exists(yiniFile))
    {
        File.Delete(yiniFile);
        Console.WriteLine($"\nDeleted temporary config file: '{yiniFile}'");
    }
}
