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
- 字符串  ->  "value"
- 整数  ->  123
- 浮点数  ->  3.14  
- 布尔值  ->  true/false  
- 数组  ->  [1, 2, 3]  
    - 二维数组使用  ->  [[1, 2], [3, 4]]  
- 坐标  ->  (x, y) / (x, y, z)
- 键值对  ->  {key: value}
- Map  ->  {{key1: value1}, {key2: value2}}
- 颜色  ->  #RRGGBB / RGB(r,g,b) / rgb(r,g,b) / 255, 192, 203
(更多类型正在添加中)  

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

```YINI
[#include]
+= file1.yini
+= file2.yini
+= file3.yini
```

按照顺序，合并同名的配置块  

### YMETA
在程序加载YINI文件之后，会为每一个YINI文件生成一个YMETA文件  
YMETA缓存着YINI文件中所有的信息，避免重复加载  
YMETA使用.ymeta，.YMETA作为后缀  

### CLI
虽然YINI是一门简单的编程语言，但也具有相关的CLI工具  
CLI工具采用阻塞式工作  
CLI工具主要用来编译与反编译YMETA文件，以及检查是否语法错误  

### VSCode IDE
没错，YINI也可以在VSCode IDE中开发  
VSCode IDE需要提供下述功能  
- 语法高亮
- 语法检查
- 代码提示
- 编译YINI文件
- 反编译YMETA文件
- 内置CLI
- 右键打开官方文档(内置官方文档)

### 项目开发建议
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
