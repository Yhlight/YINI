# YINI Unity Integration

This package provides native YINI configuration support for Unity.

## Installation

1. Copy `src/Yini/bin/Debug/net8.0/Yini.dll` to your Unity project's `Assets/Plugins` folder.
2. Copy `src/Yini.Unity` source files to `Assets/Yini`.

## Usage

### Setup
1. Create a `YiniManager` GameObject in your first scene.
2. Attach the `YiniManager` script.
3. (Optional) Assign a `DefaultConfig` asset.

### Loading Config
```csharp
YiniManager.Instance.LoadConfig(myConfigString);
```

### Accessing Values
```csharp
int health = YiniManager.Instance.Get<int>("Player", "Health", 100);
float speed = YiniManager.Instance.Get<float>("Player", "Speed", 5.0f);
```

### Dynamic Values (Dyna)
If your config uses `Dyna`, you can access runtime variables:

```ini
[Player]
Speed = Dyna(BaseSpeed * Time)
```

In code:
```csharp
YiniManager.Instance.SetVariable("BaseSpeed", new YiniFloat(10f));
// Accessing 'Speed' will evaluate (10 * Time.time)
float currentSpeed = YiniManager.Instance.Get<float>("Player", "Speed");
```

### Supported Types
- `int`, `float`, `bool`, `string`
- `Color` (mapped to `YiniColor`)
- `Vector2`, `Vector3` (mapped to `YiniCoord`)
