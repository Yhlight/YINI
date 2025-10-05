# YINI C# 使用手册

欢迎阅读 YINI C# 使用手册！本指南提供了实用的代码示例和方法，旨在帮助您在项目中高效地使用 Yini .NET 库。

## 1. 入门：加载 YINI 文件

与 YINI 交互的主要入口点是 `YiniManager` 类。首先，您需要创建该类的实例并加载您的 YINI 文件。

**示例 `settings.yini` 文件：**
```yini
[Graphics]
resolution_width = 1920
resolution_height = 1080
fullscreen = true

[Audio]
master_volume = 0.8
```

**C# 代码：**
```csharp
using Yini;
using System;

public class GameSettingsLoader
{
    public void LoadSettings()
    {
        using (var manager = new YiniManager())
        {
            if (manager.Load("path/to/settings.yini"))
            {
                Console.WriteLine("设置加载成功！");
            }
            else
            {
                Console.WriteLine("设置加载失败。");
            }
        }
    }
}
```

## 2. 读取基本类型值

`YiniManager` 提供了简单的方法来获取基本数据类型。这些方法包含一个 `defaultValue` 参数，如果未找到指定的键，则返回该默认值。

```csharp
using (var manager = new YiniManager())
{
    manager.Load("settings.yini");

    // 读取整数 (以 double 类型获取后转型)
    int width = (int)manager.GetDouble("Graphics", "resolution_width", 1280);

    // 读取布尔值
    bool isFullscreen = manager.GetBool("Graphics", "fullscreen", false);

    // 读取浮点数 (以 double 类型获取后转型)
    float volume = (float)manager.GetDouble("Audio", "master_volume", 1.0);

    // 读取一个字符串 (该键不存在，因此返回默认值)
    string playerName = manager.GetString("Player", "name", "Guest");

    Console.WriteLine($"分辨率: {width}x{1080}"); // 为简洁起见，1080 为硬编码
    Console.WriteLine($"全屏: {isFullscreen}");
    Console.WriteLine($"音量: {volume}");
    Console.WriteLine($"玩家: {playerName}");
}
```

## 3. 使用集合

YINI 让处理列表和字典变得非常简单。

**示例 `inventory.yini` 文件：**
```yini
[PlayerInventory]
items = ["Sword", "Shield", "Health Potion"]
skill_levels = { "swords": 10, "archery": 5, "magic": 2 }
```

**C# 代码：**
```csharp
using (var manager = new YiniManager())
{
    manager.Load("inventory.yini");

    // 读取字符串列表
    List<string> items = manager.GetList<string>("PlayerInventory", "items");
    if (items != null)
    {
        Console.WriteLine("物品栏: " + string.Join(", ", items));
    }

    // 读取字典
    Dictionary<string, double> skills = manager.GetDictionary<double>("PlayerInventory", "skill_levels");
    if (skills != null)
    {
        foreach (var skill in skills)
        {
            Console.WriteLine($"技能: {skill.Key}, 等级: {skill.Value}");
        }
    }
}
```

## 4. 使用源码生成器进行高性能绑定

对于性能敏感的应用程序，YINI 提供了一个源码生成器，可以在编译时创建无反射的绑定代码。这是加载复杂对象的推荐方法。

要使用此功能，请用 `[YiniBindable]` 特性标记您的 C# 类。

**示例 `player_stats.yini` 文件：**
```yini
[Stats]
name = "Jules"
level = 15
is_active = true
hit_points = 95.5
```

**C# 类定义：**
```csharp
using Yini;

// 此特性会告知源码生成器为此类创建绑定器。
[YiniBindable]
public partial class PlayerStats
{
    // [YiniKey] 特性可用于将属性映射到不同的键名。
    // 如果省略，则使用属性名（转换为小写蛇形命名法）作为键。
    [YiniKey("name")]
    public string PlayerName { get; set; } = "Default";

    public int Level { get; set; } = 1;

    [YiniKey("is_active")]
    public bool IsActive { get; set; }

    // 此属性不会被绑定，因为它在 YINI 文件中缺少对应的键。
    // 它的默认值将被保留。
    public float Mana { get; set; } = 100.0f;

    // 属性会自动映射到键，例如，HitPoints -> hit_points
    public double HitPoints { get; set; }
}
```

**C# 使用方法：**
```csharp
using (var manager = new YiniManager())
{
    manager.Load("player_stats.yini");

    var stats = new PlayerStats();

    // 调用源码生成的 `BindFromYini` 方法以实现高性能绑定。
    stats.BindFromYini(manager, "Stats");

    Console.WriteLine($"名称: {stats.PlayerName}");       // 输出: Jules
    Console.WriteLine($"等级: {stats.Level}");          // 输出: 15
    Console.WriteLine($"激活状态: {stats.IsActive}");   // 输出: True
    Console.WriteLine($"法力值: {stats.Mana}");            // 输出: 100.0 (保留默认值)
    Console.WriteLine($"生命值: {stats.HitPoints}"); // 输出: 95.5
}
```
这种方法避免了反射的开销，使其成为在启动或游戏过程中加载配置数据的理想选择。