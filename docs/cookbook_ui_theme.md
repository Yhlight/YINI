# Cookbook: Defining a UI Theme

A powerful use case for YINI in game development is defining a theme for your user interface. By centralizing colors, fonts, and other style properties, you can ensure a consistent look and feel across your entire game and make sweeping visual changes by editing just one file.

This recipe demonstrates how to create a simple UI theme using YINI, showcasing macros, color values, and section organization.

## The Goal

We want to define a theme with:
- A primary and secondary color palette.
- A default font and a title font.
- Specific styles for different UI elements like buttons and text labels.

## The YINI File: `ui_theme.yini`

```yini
// ui_theme.yini
// Central definition for all UI styling in the game.

[#define]
// Color Palette
primary_color = Color(50, 150, 250)   // A nice blue
secondary_color = Color(200, 200, 200) // Light gray
text_color_light = Color(240, 240, 240)
text_color_dark = Color(30, 30, 30)
danger_color = Color(255, 80, 80)

// Font Definitions
default_font_face = "Roboto"
title_font_face = "RobotoSlab"

// --- UI Element Styles ---

[Button_Default]
background_color = @primary_color
text_color = @text_color_light
font_face = @default_font_face
font_size = 16
padding = Vec2(10, 5)

// A danger button inherits from the default button but overrides the color.
[Button_Danger] : Button_Default
background_color = @danger_color

[Label_Default]
text_color = @text_color_dark
font_face = @default_font_face
font_size = 14

[Label_Title] : Label_Default
font_face = @title_font_face
font_size = 28
text_color = @primary_color
```

## How It Works

1.  **`[#define]` Block:** We define all our core theme properties as macros. This is incredibly powerful. If you want to change your game's primary color, you only need to edit the `@primary_color` macro, and every UI element that references it will be updated automatically.

2.  **`Color()` and `Vec2()`:** We use YINI's built-in functions to define complex data types like colors (with RGB values) and 2D vectors for padding. This makes the configuration much more readable than using raw numbers or strings.

3.  **Section-Based Styles:** Each UI element style (e.g., `Button_Default`, `Label_Title`) is defined in its own section. This keeps the configuration clean and organized. Your UI system can then load the style for a specific element by requesting its section.

4.  **Inheritance for Variations:** The `[Button_Danger] : Button_Default` syntax is a great example of YINI's inheritance. The "Danger" button automatically gets all the properties of the "Default" button (`text_color`, `font_face`, etc.) but overrides the `background_color` to use the `@danger_color` macro. This reduces duplication and makes creating variations simple and maintainable.

## Usage in C#

You could load this theme into a C# object using YINI's binding capabilities.

```csharp
// Define C# classes to hold the style data
public class ButtonStyle
{
    [YiniKey("background_color")]
    public Color BackgroundColor { get; set; }

    [YiniKey("text_color")]
    public Color TextColor { get; set; }

    [YiniKey("font_face")]
    public string FontFace { get; set; }
    // ... other properties
}

// In your UI Manager
var yini = new YiniManager();
yini.Load("ui_theme.yini");

// Bind the styles to your objects
var defaultButtonStyle = yini.Bind<ButtonStyle>("Button_Default");
var dangerButtonStyle = yini.Bind<ButtonStyle>("Button_Danger");

// Now you can use these strongly-typed objects to configure your UI elements.
```

This cookbook recipe shows how YINI's features work together to create a configuration system that is powerful, readable, and easy to maintain, making it a perfect fit for managing the complexities of a modern game UI.