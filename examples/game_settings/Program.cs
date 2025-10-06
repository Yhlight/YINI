using System;
using System.IO;
using Yini;

// Define a class to hold graphics settings for data binding.
public class GraphicsSettings
{
    [YiniKey("resolution_x")]
    public int Width { get; set; }

    [YiniKey("resolution_y")]
    public int Height { get; set; }

    public bool Fullscreen { get; set; }

    public bool VSync { get; set; }

    public string Quality { get; set; } = "Medium";
}

public class Program
{
    public static void Main()
    {
        string filePath = "settings.yini";
        if (!File.Exists(filePath))
        {
            Console.WriteLine($"Error: '{filePath}' not found. Make sure you are running this from the 'examples/game_settings' directory.");
            return;
        }

        using var manager = new YiniManager();

        Console.WriteLine($"Loading settings from '{filePath}'...");
        manager.Load(filePath);

        // 1. Validate the file against its schema
        Console.WriteLine("\n--- Schema Validation ---");
        var validationErrors = manager.Validate();
        if (validationErrors.Any())
        {
            Console.WriteLine("Validation failed:");
            foreach (var error in validationErrors)
            {
                Console.WriteLine($"- {error}");
            }
        }
        else
        {
            Console.WriteLine("Settings are valid according to the schema.");
        }

        // 2. Retrieve settings using the generic Get<T> method
        Console.WriteLine("\n--- Retrieving Individual Settings ---");
        int width = manager.Get<int>("Graphics", "resolution_x");
        int height = manager.Get<int>("Graphics", "resolution_y");
        float masterVolume = manager.Get<float>("Audio", "master_volume");

        Console.WriteLine($"Resolution: {width}x{height}");
        Console.WriteLine($"Master Volume: {masterVolume:P0}"); // Format as percentage

        // 3. Use reflection-based data binding to load a section into an object
        Console.WriteLine("\n--- Data Binding ---");
        var graphics = manager.Bind<GraphicsSettings>("Graphics");
        Console.WriteLine($"Graphics Quality (from bound object): {graphics.Quality}");
        Console.WriteLine($"VSync Enabled (from bound object): {graphics.VSync}");

        // 4. Modify a dynamic value and save it
        Console.WriteLine("\n--- Modifying and Saving ---");
        float newVolume = 0.65f;
        Console.WriteLine($"Changing master volume from {masterVolume:P0} to {newVolume:P0}...");
        manager.SetDouble("Audio", "master_volume", newVolume);

        // SaveChanges performs a non-destructive write-back, preserving comments.
        manager.SaveChanges();
        Console.WriteLine("Changes saved to 'settings.yini'.");

        Console.WriteLine("\nRe-loading to verify change...");
        using var manager2 = new YiniManager();
        manager2.Load(filePath);
        float savedVolume = manager2.Get<float>("Audio", "master_volume");
        Console.WriteLine($"New volume from file: {savedVolume:P0}");

        if (Math.Abs(savedVolume - newVolume) < 0.001)
        {
             Console.WriteLine("Successfully verified the saved change!");
        }
        else
        {
             Console.WriteLine("Error: The change was not saved correctly.");
        }
    }
}