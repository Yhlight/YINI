## YINI配置文件
YINI是一款配置文件语言，专用于2D游戏开发，基于C#进行编写  
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
- 颜色  ->  #RRGGBB / Color(255, 192, 203)  
- 坐标  ->  Coord(x, y) / Coord(x, y, z)
- 路径  ->  Path()  
- 链表  ->  List(1, 2, 3)  // 默认情况下，[] 表示数组，为了解决不能使用链表的问题，这里引入了显性的表示  
- 数组  ->  Array(1, 2, 3)  
(更多类型正在添加中)  

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
