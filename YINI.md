## YINI配置文件
常规的INI必然无法支持我们的游戏的开发  
为此，我进行了语法的扩展  
我们使用.yini或.YINI作为配置文件的后缀  

## 注释
在YINI中，使用//和/* */进行注释  

## 继承
现在你可以在配置块头部处使用:继承其他的配置块  

```YINI
[Config]
key1 = value1
key2 = value2

[Config2]
key3 = value3
key4 = value4

[Config3] : Config, Config2
```
按照顺序，继承其中的键值对，后继承的配置块覆盖前继承或已经存在的键值对  

## 快捷注册
在游戏开发中，配置文件往往管理着游戏中某一个物件的注册表  
这些注册表通常需要使用使用明确的序号表示索引，这极其影响开发效率  
现在你可以使用`+=`来快捷注册  

```YINI
[Reg]
+= value1
+= value2
+= value3
```

### 值
YINI并不将所有的配置值作为字符串  
而是根据游戏所需要的值类型，直接将其转换为对应的值类型  
YINI支持多种值类型  
分别为  
- 整数  ->  123  
- 浮点数  ->  3.14  
- 布尔值  ->  true/false  
- 字符串  ->  "value"  
- 数组  ->  [1, 2, 3]  
    - 二维数组使用  ->  [[1, 2], [3, 4]]  
- 集合 -> (value, value, value3)  
            - (value, ) 只有一个元素集合需要需要添加逗号  
- 结构体  ->  {key: value}  // 我不觉得使用一个Map来存储一个对组是一件好事，数量太少了，浪费性能  
- Map(对组的集合)  ->  {key1: value1, key2: value2}  
- 颜色  ->  #RRGGBB / color(255, 192, 203) / Color(255, 192, 203)  
- 坐标  ->  Coord(x, y) / Coord(x, y, z) / coord(x, y) / coord(x, y, z)  
- 路径  ->  path() / Path()  
- 链表  ->  List(1, 2, 3) / list(1, 2, 3)  // 默认情况下，[] 表示数组，为了解决不能使用链表的问题，这里引入了显性的表示  
- 数组  ->  Array(1, 2, 3) / array(1, 2, 3)  
(更多类型正在添加中)  

#### 动态
原生INI中，键值对只能是静态的，不变的  
YINI中，你可以使用Dyna() / dyna()封装键值  
这个键会在随游戏动态更新，Dyna()具有实时更新以及缓存更新两种更新方式  
两者配合使用，不可选  
当键值被修改时，键值会被写进YMETA文件之中  
在游戏退出时，或游戏重新启动时，会从YMETA文件中提取键值，然后更新YINI文件  
Dyna()需要包装值进行使用  

```YINI
[Config]
key = Dyna(1)  // 这个键会随游戏而动态更新
```

##### backup
为了防止Dyna()错误覆写了键值，YMETA(YINI元数据，详细见下文)文件会缓存Dyna()的键的键值，缓存次数不超过五次  

### 算术运算
YINI支持常见的算术运算  
包括 + - * / %  
可以使用括号进行优先级的控制  
仅限于基本数据类型  
算术运算仅支持宏与基本数据类型的运算  

### 宏定义和变量引用
[#define]
你可以在[#define]之中定义宏  
然后在代码中使用@name引用  

```YINI
[#define]
name = value

[UI]
UIName = @name
```

### 文件关联
现在，你可以使用[#include]关联其他文件  
这些文件将根据顺序决定覆写顺序  
[#include]无需使用path，直接写路径即可  

```YINI
[#include]
+= file1.yini
+= file2.yini
+= file3.yini
```

按照顺序，合并同名的配置块(不存在的添加，存在的覆写)，[#define]块也遵守这样的行为  

### 配置块验证
你可以使用[#schema]对配置块进行验证  
验证块通常会在生成YMETA文件之前进行  
确保你的文件之中包含此配置块  

验证条件有下述形式  
是否必须  
必须(!)，可选(?)  

键值类型  
int, float, bool, string, array, list, map, color, coord, path  
可以指定容器的容纳类型array[int], array[array[int]]  

空值行为  
忽略(~)，赋予值(=)，抛出错误(e)  

范围验证  
min=, max=  
范围为[min, max]  

使用方式如下，`!, int, =1280`  
不写其中的某一个选项则是忽略此类型的验证  

```YINI
[#schema]
[Visual]
width = !, int, =1280, min=800, max=1920
height = ?, int  // 不进行空值验证
fps = ?, int, =60
isOld = !, bool, e

[Config]

[#schema]
[Audio]
volume = !, float, =1.0
music = ?, bool, =true
```

#### 实践
一般情况下，不建议在同一个文件之中将验证块与配置块放一起  
而是应该分开  

schema.yini  
```YINI
[#schema]
[Visual]
width = !, int, =1280, min=800, max=1920
height = ?, int
fps = ?, int, =60
isOld = !, bool, e
```

config.yini  
```YINI
[#include]
+= schema.yini  // 引用schema

[Visual]
width = 1920
height = 1080
fps = 60
isOld = true
```

### 环境变量
YINI支持环境变量，你可以使用${name}引用环境变量  

```YINI
[Visual]
width = ${WIDTH}
height = ${HEIGHT}
```

### 横截面引用
YINI支持横截面引用，你可以使用@{name}引用横截面变量  

```YINI
[Config]
width = 1920
height = 1080

[Visual]
width = @{Config.width}
height = @{Config.height}
```

### YMETA
在程序加载YINI文件之后，会为每一个YINI文件生成一个YMETA文件  
YMETA缓存着YINI文件中所有的信息，避免重复加载  
YMETA使用.ymeta，.YMETA作为后缀  

对于具有动态值DYNA()的YMETA文件，它们会在游戏中持续存在，其他则不会  

### CLI
虽然YINI是一门简单的编程语言，但也具有相关的CLI工具  
CLI工具需要采用阻塞式运行，意思是，CLI需要存在事件主进程事件循环，持续交互  
CLI工具主要用来编译与反编译YMETA文件，以及检查是否语法错误  

### 综合示例 (Examples)
下面的示例展示了如何结合使用多种YINI功能 (继承、宏、模式验证和多样的数据类型) 来创建一个更复杂和健壮的游戏配置。

#### 示例：定义怪物模板和具体的怪物类型

在这个场景中，我们首先定义一个 schema 文件来确保所有怪物都有共同的属性，然后我们创建一个基础的怪物 "template"，最后定义两个继承自该模板的具体怪物。

**`schemas/monsters.yini`**
```YINI
[#schema]
[BaseMonster]
name = !, string, e
health = !, int, min=1
mana = ?, int, =0
speed = !, float
abilities = ?, array[string]

[Goblin]
loots = !, array[string]

[Dragon]
fire_breath_damage = !, int, min=50
```

**`configs/monsters.yini`**
```YINI
[#include]
+= "../schemas/monsters.yini"

[#define]
common_loot = "gold coins"

// 基础怪物模板 (不会直接在游戏中使用)
[BaseMonster]
name = "Unnamed Monster"
health = 100
speed = 5.0
abilities = ["attack", "block"]

// 一个具体的怪物类型，继承自基础模板
[Goblin] : BaseMonster
name = "Goblin Grunt"
health = 50
speed = 7.5
abilities = ["attack", "dodge"]
loots = [@common_loot, "rusty dagger"]

// 另一个具体的怪物类型，具有独特的属性
[Dragon] : BaseMonster
name = "Ancient Dragon"
health = 2000
speed = 15.0
abilities = ["attack", "fly", "fire_breath"]
fire_breath_damage = 100
```

在这个示例中：
- **`[#include]`** 用于首先加载 schema 以确保后续的配置块都将被验证。
- **`[#define]`** 定义了一个通用的宏 `@common_loot`，它可以在多个地方被重用。
- **继承** 被用来创建一个 `Goblin` 和一个 `Dragon`，它们都继承了 `BaseMonster` 的基本属性。注意 `[Goblin]` 和 `[Dragon]` 如何覆写了 `name` 和 `health` 等属性。
- **数据类型** 展示了字符串 (`string`)，整数 (`int`)，浮点数 (`float`) 和字符串数组 (`array[string]`) 的使用。
- **Schema 验证** 将确保：
  - `Goblin` 必须有一个 `loots` 数组。
  - `Dragon` 必须有一个 `fire_breath_damage` 并且其值不能小于50。
  - 所有怪物都必须有 `name`, `health` 和 `speed`。
  - 如果 `mana` 没有被提供，它将默认为0。

### 项目开发建议
#### 架构设计
状态机 + 策略模式 / 递归下降  

#### 项目命名规范
基本数据类型，常规函数  ->  蛇形命名法  
类的成员变量(基本数据类型)  ->  蛇形命名法  
类的成员变量(非基本数据类型)  ->  小驼峰命名法  
类的成员函数  ->  小驼峰命名法  
与类有关的变量  ->  小驼峰命名法  
数据结构  ->  大驼峰命名法  

#### 代码风格
括号风格  ->  Allman  

#### 目录结构
YINI
    |- src
        |- Lexer
        |- Parser
        |- CLI
    |- docs
