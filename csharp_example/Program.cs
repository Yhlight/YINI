using System;
using System.Text;
using Yini;

// This example demonstrates how to use the YINI C# wrapper.
// It will create a test .yini file, load it using the native library,
// read values from it, and print them to the console.

// --- 1. Create a test YINI file ---
const string yiniFile = "test.yini";
File.WriteAllText(yiniFile, @"
[Player]
name = ""Jules""
level = 99
hp_percent = 98.6
is_online = true
");

Console.WriteLine("--- C# YINI P/Invoke Example ---");
Console.WriteLine($"Created temporary config file: '{yiniFile}'");

IntPtr yiniHandle = IntPtr.Zero;
try
{
    // --- 2. Load the YINI file using the native library ---
    yiniHandle = Native.Load(yiniFile);
    if (yiniHandle == IntPtr.Zero)
    {
        throw new InvalidOperationException("Failed to load YINI file. See console for C++ errors.");
    }
    Console.WriteLine("Successfully loaded YINI file with handle.");

    // --- 3. Retrieve values ---
    Console.WriteLine("\nReading values from [Player] section:");

    // Get String
    StringBuilder nameBuffer = new StringBuilder(256);
    if (Native.GetString(yiniHandle, "Player", "name", nameBuffer, nameBuffer.Capacity) != -1)
    {
        Console.WriteLine($"  - Name: {nameBuffer}");
    }
    else
    {
        Console.WriteLine("  - Failed to get 'name'.");
    }

    // Get Int64
    if (Native.GetInt64(yiniHandle, "Player", "level", out long level) == 1)
    {
        Console.WriteLine($"  - Level: {level}");
    }
    else
    {
        Console.WriteLine("  - Failed to get 'level'.");
    }

    // Get Double
    if (Native.GetDouble(yiniHandle, "Player", "hp_percent", out double hp) == 1)
    {
        Console.WriteLine($"  - HP Percent: {hp}");
    }
    else
    {
        Console.WriteLine("  - Failed to get 'hp_percent'.");
    }

    // Get Bool
    if (Native.GetBool(yiniHandle, "Player", "is_online", out bool isOnline) == 1)
    {
        Console.WriteLine($"  - Is Online: {isOnline}");
    }
    else
    {
        Console.WriteLine("  - Failed to get 'is_online'.");
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
    // --- 4. Free the native library handle ---
    if (yiniHandle != IntPtr.Zero)
    {
        Native.Free(yiniHandle);
        Console.WriteLine("\nFreed YINI handle.");
    }

    // Clean up the temporary file
    File.Delete(yiniFile);
    Console.WriteLine($"Deleted temporary config file: '{yiniFile}'");
}
