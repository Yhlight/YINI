# YINI Godot Integration

This package provides native YINI configuration support for Godot 4.x (C#).

## Installation

1. Include `Yini.dll` in your Godot project (or reference the `Yini` project).
2. Add `YiniRuntime.cs` to your project autoloads (Project Settings -> Autoload). Name it `Yini`.

## Usage

### Loading Config
```csharp
YiniRuntime.Instance.LoadConfig("res://config.yini");
```

### Accessing Values
```csharp
int health = YiniRuntime.Instance.Get("Player", "Health", 100).AsInt32();
float speed = (float)YiniRuntime.Instance.Get("Player", "Speed", 5.0f);
```

### Dynamic Values
YINI Runtime automatically provides `Time` variable.

```ini
[Animation]
Offset = Dyna(Time * 2.0)
```

You can resolve this value at runtime:
```csharp
var offset = YiniRuntime.Instance.Get("Animation", "Offset");
```
