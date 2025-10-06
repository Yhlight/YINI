# YINI C# Source Generator Guide

The YINI library includes a powerful C# Source Generator to provide high-performance, reflection-free data binding. This guide explains how to use it and the benefits it provides over the reflection-based `Bind<T>` method.

## 1. What is the Source Generator?

The source generator is a compile-time tool that automatically creates C# code for you. In YINI, it looks for classes marked with the `[YiniBindable]` attribute and generates a `BindFromYini` method for them.

This method populates an object's properties from a YINI section without using reflection, which results in significantly better performance and makes your code AOT (Ahead-Of-Time) compilation friendly, which is important for platforms like iOS, Android, and game consoles.

## 2. How to Use It

Using the source generator is simple. Just add the `[YiniBindable]` attribute to any class you want to be able to bind from a YINI file.

**Step 1: Define Your Configuration Class**

Create a standard C# class (a POCO) to hold your configuration data.

```csharp
// In your project, for example in a file named `PlayerConfig.cs`

using Yini; // You need to import the Yini namespace

[YiniBindable]
public partial class PlayerConfig // Note: The class must be marked as 'partial'
{
    // Public properties with getters and setters
    public string Name { get; set; } = "Default Player";
    public int Level { get; set; }
    public float Health { get; set; } = 100f;
    public bool IsActive { get; set; }
    public List<string> Inventory { get; set; }
}
```

**Important:**
*   The class **must** be marked as `partial`. This is because the source generator creates another part of the class in a separate, auto-generated file.
*   The `[YiniBindable]` attribute is automatically available when you reference the YINI library; you don't need to install any other packages.

**Step 2: Use the Generated Method**

Once your class is marked, the source generator creates a `BindFromYini` method on it. You can then call this method to populate an instance of your class.

**config.yini:**
```yini
[Player]
name = "Jules"
level = 25
is_active = true
```

**C# Binding Code:**
```csharp
using (var yini = new YiniManager())
{
    yini.Load("config.yini");

    // Create an instance of your config class
    var playerConfig = new PlayerConfig();

    // The 'Health' property will remain 100f because it's not in the YINI file.
    Console.WriteLine($"Health before binding: {playerConfig.Health}");

    // Call the generated method to populate the object
    playerConfig.BindFromYini(yini, "Player");

    Console.WriteLine($"Name: {playerConfig.Name}");     // Output: Jules
    Console.WriteLine($"Level: {playerConfig.Level}");    // Output: 25
    Console.WriteLine($"IsActive: {playerConfig.IsActive}"); // Output: true
    Console.WriteLine($"Health after binding: {playerConfig.Health}"); // Output: 100
}
```

## 3. Key Mapping Rules

The generator maps YINI keys to C# properties using the following rules:

1.  **Default Convention:** By default, `PascalCase` property names are converted to `snake_case` key names.
    *   `PlayerName` -> `player_name`
    *   `ResolutionX` -> `resolution_x`

2.  **Custom Mapping with `[YiniKey]`:** If your YINI key doesn't follow the convention, you can specify it explicitly using the `[YiniKey]` attribute (the same one used by the reflection-based `Bind<T>`).

    ```csharp
    [YiniBindable]
    public partial class GraphicsConfig
    {
        [YiniKey("resolution-width")] // Custom key name
        public int Width { get; set; }
    }
    ```

## 4. Preserving Default Values

A key feature of the source generator is that it **will not overwrite** property values if the corresponding key is missing from the YINI file.

In the example above, the `PlayerConfig` class initializes the `Health` property to `100f`. Since the `[Player]` section in `config.yini` does not contain a `health` key, the `playerConfig.Health` property remains `100f` after binding.

This is a significant advantage, as it allows you to define safe, default values directly in your C# code.

## 5. Benefits vs. Reflection-Based `Bind<T>`

| Feature                 | Source-Generated `BindFromYini`                               | Reflection-Based `yini.Bind<T>`                          |
|-------------------------|---------------------------------------------------------------|----------------------------------------------------------|
| **Performance**         | **High.** No reflection overhead. Direct method calls.        | **Lower.** Incurs reflection overhead at runtime.        |
| **AOT Compatibility**   | **Yes.** Fully compatible with AOT compilation.               | **No.** Reflection can cause issues with AOT platforms.  |
| **Setup**               | Requires class to be `partial` and have `[YiniBindable]`.     | No special requirements for the class.                   |
| **Instantiation**       | Populates an *existing* instance.                             | Creates a *new* instance.                                |
| **Preserving Defaults** | **Yes.** Keys not found in YINI do not overwrite C# defaults. | **Yes.** Behavior is consistent.                         |

**Recommendation:** Use the source generator (`[YiniBindable]`) whenever possible, especially in performance-sensitive contexts or when targeting AOT platforms. Use the reflection-based `Bind<T>` for quick prototyping or in less performance-critical parts of your application where modifying the class definition is inconvenient.