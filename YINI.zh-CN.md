# YINI 语言手册

YINI 是一种为游戏开发等性能关键型应用设计的现代配置文件格式。它扩展了我们熟悉的 INI 语法，并提供了一套强大的功能，包括丰富的类型系统、分区继承、宏、模式验证以及与 C# 的无缝集成。

## 1. 核心概念

### 1.1. 注释

YINI 支持两种注释风格：

-   **单行注释:** 以 `//` 开始，直到行尾。
-   **多行注释:** 包含在 `/*` 和 `*/` 之间。

```yini
// 这是一个单行注释。

/*
  这是一个
  多行注释。
*/
```

### 1.2. 分区与键

配置被组织在分区（sections）中，分区由 `[]` 中的名称定义。每个分区包含若干键值对。

```yini
[Player]
name = "Jules"
level = 10
```

## 2. 数据类型

YINI 支持丰富的数据类型集。

| 类型      | 示例                                             | 描述                                           |
| :-------- | :----------------------------------------------- | :--------------------------------------------- |
| **整数**  | `42`                                             | 一个整数。                                     |
| **浮点数**| `3.14`                                           | 一个浮点数。                                   |
| **布尔值**| `true` 或 `false`                                | 一个布尔值。                                   |
| **字符串**| `"Hello, World!"` 或 `'Hello, World!'`           | 由单引号或双引号包围的文本。                   |
| **数组**  | `[1, "two", 3.0, true]`                          | 一个有序的、可混合类型的列表。                 |
| **集合**  | `(1, "two", 3.0, true)`                          | 一个有序的、可混合类型的列表，类似于数组。     |
| **映射**  | `{ "key1": "value1", "key2": 123 }`              | 键值对的集合。键必须是字符串。                 |
| **颜色**  | `Color(255, 192, 203)` 或 `Color(255,192,203,128)` | 代表一个 RGB 或 RGBA 颜色。                    |
| **向量**  | `Vec2(x, y)`, `Vec3(x, y, z)`, `Vec4(x, y, z, w)` | 代表 2D, 3D, 或 4D 坐标。                      |
| **路径**  | `Path("/path/to/file")`                          | 代表一个文件系统路径。                         |
| **动态值**| `Dyna(initial_value)`                            | 一个可以在运行时更改并保存回文件中的值。       |

## 3. 语言特性

### 3.1. 值注册

`+=` 运算符提供了一种便捷的方式来在单个键下构建一个值列表，最终形成一个数组。

```yini
[Weapons]
available += "Sword"
available += "Axe"
available += "Bow"

// 'available' 键现在等同于：
// available = ["Sword", "Axe", "Bow"]
```

### 3.2. 分区继承

一个分区可以从一个或多个父分区继承键和值。后继承的父分区或子分区自己的值将覆盖先继承的父分区的值。

```yini
[BaseCharacter]
health = 100
mana = 50

[Warrior] : BaseCharacter
mana = 20 // 覆盖了 BaseCharacter 中的 mana
strength = 15

// 'Warrior' 分区现在实际包含：
// health = 100, mana = 20, strength = 15
```

### 3.3. 宏 (`[#define]`)

`[#define]` 分区允许你声明可在整个文件中重用的常量。使用 `@` 符号来引用一个宏。

```yini
[#define]
primary_color = Color(255, 0, 0)
base_damage = 10

[UI]
button_color = @primary_color

[Player]
attack = @base_damage * 1.5
```

### 3.4. 文件包含 (`[#include]`)

使用 `[#include]` 分区将你的配置分割到多个文件中。路径是相对于当前文件的。

```yini
[#include]
+= "settings.yini"
+= "player_stats.yini"
```

### 3.5. 算术运算

YINI 支持基本的算术运算 (`+`, `-`, `*`, `/`, `%`)。使用括号 `()` 进行分组以控制运算顺序。

```yini
[Calculations]
value = (10 + 5) * 2 // 结果是 30
```

### 3.6. 高级引用

#### 跨分区引用
使用 `@{section.key}` 语法来引用另一个分区中的值。

```yini
[Settings]
master_volume = 0.8

[Audio]
// 引用 Settings 分区中的 master_volume
music_volume = @{Settings.master_volume} * 0.5
```

#### 环境变量
使用 `${VAR_NAME}` 语法从系统环境变量中获取值。你也可以提供一个默认值，格式为 `${VAR_NAME:default}`。

```yini
[Database]
// 使用 "DB_HOST" 环境变量的值，如果未设置则使用 "localhost"。
host = ${DB_HOST:localhost}
port = ${DB_PORT:5432}
```

## 4. 动态值与非破坏性保存

需要在运行时更改的值（例如，玩家生命值、设置）可以使用 `Dyna()` 标记为动态值。

```yini
[Player]
// 这个值可以在代码中更新并保存。
health = Dyna(100)
```

当你保存更改时，YINI 会智能地只更新原始文件中被修改的值，并保留所有的注释和格式。这使得手动和程序化地管理配置文件都变得非常安全。

## 5. 模式与验证 (`[#schema]`)

YINI 允许你在配置文件中直接嵌入一个模式（schema），以强制执行特定的结构。这是一个可选但非常强大的功能，可以确保配置的正确性。

`[#schema]` 块定义了预期的分区和键，以及它们的类型和是否为必需项。

```yini
[#schema]
[Player]
name: string!      // 必需的字符串
class: string      // 可选的字符串
level: integer!    // 必需的整数
inventory: [item]  // 可选的 item 类型数组（定义如下）

[item]
id: string!
quantity: integer
```

你可以使用命令行工具来根据其内嵌的模式验证一个文件。

## 6. 命令行界面 (CLI)

YINI 提供了一个命令行工具 (`yini-cli`) 来执行常用操作。

| 命令                          | 描述                                                     |
| :---------------------------- | :------------------------------------------------------- |
| `check <filepath>`            | 检查 `.yini` 文件的语法错误。                            |
| `validate <filepath>`         | 根据文件内嵌的 `[#schema]` 来验证文件。                  |
| `compile <in.yini> <out.ymeta>`| 将 `.yini` 文件编译成二进制的 `.ymeta` 格式。            |
| `decompile <in.ymeta>`        | 反编译 `.ymeta` 文件并将其内容打印到控制台。             |

## 7. C# / .NET 集成

YINI 提供了一个高性能的 C# 库，可以无缝集成到 .NET 项目中。

### 7.1. 基本用法

`YiniManager` 类是主要的入口点。

```csharp
using var manager = new YiniManager();
manager.Load("config.yini");

// 获取值
string name = manager.GetString("Player", "name", "Default");
double health = manager.GetDouble("Player", "health", 100.0);
bool active = manager.GetBool("Player", "is_active", false);

// 设置一个动态值并保存
manager.SetDouble("Player", "health", 85.5);
manager.SaveChanges();
```

### 7.2. 复杂类型

YINI 提供了便捷的方法来获取集合和特殊类型。

```csharp
// 获取一个字符串列表
List<string> items = manager.GetList<string>("Inventory", "items");

// 获取一个字典
Dictionary<string, int> ammo = manager.GetDictionary<int>("Inventory", "ammo");

// 获取特殊的向量类型 (需要 System.Numerics)
Vector3 position = manager.Get<Vector3>("Player", "position");

// 获取一个颜色类型
Color color = manager.Get<Color>("UI", "theme_color");
```

### 7.3. 数据绑定

YINI 提供了两种方式将配置数据直接绑定到 C# 对象上。

#### 基于反射的绑定
`Bind<T>` 方法提供了一种简单的方式来填充对象的属性。如果属性名与蛇形命名（snake_case）的键名不匹配，可以使用 `[YiniKey]` 特性。

```csharp
public class GraphicsSettings
{
    [YiniKey("resolution_x")]
    public int Width { get; set; }

    [YiniKey("resolution_y")]
    public int Height { get; set; }

    public bool VSync { get; set; } // 会自动映射到 "v_sync"
}

// ...
GraphicsSettings settings = manager.Bind<GraphicsSettings>("Graphics");
```

#### 高性能的源生成绑定
为了实现零反射的最高性能，可以用 `[YiniBindable]` 特性来注解你的类。这会在编译时生成一个 `BindFromYini` 方法。

- `PascalCase` 属性会自动转换为 `snake_case` 键名。
- `[YiniKey]` 特性可以用来覆盖默认的名称。

```csharp
[YiniBindable]
public partial class PlayerStats
{
    public string PlayerName { get; set; } // 映射到 "player_name"

    [YiniKey("character_class")]
    public string Class { get; set; }
}

// ...
var stats = new PlayerStats();
// 这个生成的方法速度极快！
stats.BindFromYini(manager, "Player");
```