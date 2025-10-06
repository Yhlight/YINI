# YINI - 现代化游戏配置语言

YINI 是一门基于C++17实现的现代化配置语言，专为游戏开发设计，是对传统INI配置文件的全面升级。

## 特性

### 核心功能
- ✅ **注释支持**: 使用 `//` 和 `/* */` 进行代码注释
- ✅ **配置块继承**: 支持多重继承，实现配置复用
- ✅ **快捷注册**: 使用 `+=` 快速创建列表项
- ✅ **丰富的数据类型**: 整数、浮点数、布尔值、字符串、数组、Map等
- ✅ **算术运算**: 支持基本算术表达式 (+, -, *, /, %)
- ✅ **宏定义**: 使用 `[#define]` 定义常量，通过 `@name` 引用
- ✅ **文件包含**: 使用 `[#include]` 组织多个配置文件
- ✅ **环境变量**: 使用 `${NAME}` 引用环境变量
- ✅ **跨段引用**: 使用 `@{section.key}` 引用其他配置段的值

### 特殊数据类型
- **颜色**: `#RRGGBB` 或 `Color(r, g, b)`
- **坐标**: `Coord(x, y)` 或 `Coord(x, y, z)`
- **路径**: `Path("path/to/file")`
- **链表**: `List(1, 2, 3)`
- **动态值**: `Dyna(value)` - 运行时可更新

### IDE支持 (v1.3.0新增)
- ✅ **C++原生LSP服务器**: 零依赖，高性能
- ✅ **实时语法检测**: 边输入边检查
- ✅ **智能自动补全**: 宏引用、横截面引用、类型提示
- ✅ **VSCode集成**: 完整的IDE体验

## 项目结构

```
YINI/
├── src/
│   ├── Lexer/       # 词法分析器
│   ├── Parser/      # 递归下降解析器
│   ├── CLI/         # 命令行工具
│   └── LSP/         # LSP服务器 (v1.3.0新增)
├── include/         # 头文件
├── tests/           # 单元测试
├── examples/        # 示例配置文件
├── vscode-plugin/   # VSCode扩展
├── docs/            # 文档
└── build/           # 构建目录

```

## 快速开始

### 构建项目

```bash
# 使用Python构建脚本（推荐）
./build.py --clean --test

# 构建产物:
# - build/bin/yini_cli  (CLI工具)
# - build/bin/yini_lsp  (LSP服务器)
# - build/lib/libyini.so (共享库)
```

### 使用LSP服务器 (新增)

```bash
# 1. 安装LSP服务器
sudo cp build/bin/yini_lsp /usr/local/bin/

# 2. 安装VSCode扩展
cd vscode-plugin
npm install

# 3. 在VSCode中打开.yini文件
# 享受自动补全、错误检测等IDE功能！
```

### 构建选项

```bash
# Debug构建
./build.py --build-type Debug

# Release构建
./build.py --build-type Release

# 仅配置
./build.py --configure-only

# 运行测试
./build.py --test

# 并行构建
./build.py --jobs 4
```

## 使用示例

```ini
// 定义常量
[#define]
SCREEN_WIDTH = 1920
SCREEN_HEIGHT = 1080

// 基础配置
[Graphics]
width = @SCREEN_WIDTH
height = @SCREEN_HEIGHT
fullscreen = true
bg_color = #000000

// 继承配置
[GraphicsLow] : Graphics
width = 1280
height = 720

// 数组和集合
[Inventory]
items = ["Sword", "Shield", "Potion"]
quantities = [1, 1, 5]

// 算术运算
[Derived]
half_width = @SCREEN_WIDTH / 2
aspect_ratio = @SCREEN_WIDTH / @SCREEN_HEIGHT

// 动态值
[GameState]
player_level = Dyna(1)
score = Dyna(0)
```

## 架构设计

- **词法分析器 (Lexer)**: 手动实现的词法分析器，负责将源代码分解为词法单元（tokens）。
- **递归下降解析器 (Recursive Descent Parser)**: 解析器采用经典的递归下降策略。
  - **Pratt解析**: 为了优雅地处理算术表达式，解析器内部实现了一个Pratt解析器，这是一种高效处理操作符优先级的技术。

### 命名规范

- 基本数据类型、常规函数: 蛇形命名法 (snake_case)
- 类成员变量(基本类型): 蛇形命名法
- 类成员变量(非基本类型): 小驼峰命名法 (camelCase)
- 类成员函数: 小驼峰命名法
- 数据结构/类: 大驼峰命名法 (PascalCase)

### 代码风格

- 括号风格: Allman
- C++标准: C++17
- 错误处理: 异常 + 错误信息

## 测试

项目使用TDD (测试驱动开发) 方式开发，包含完整的单元测试：

```bash
# 运行所有测试
./build.py --test

# 单独运行Lexer测试
./build/bin/test_lexer

# 单独运行Parser测试
./build/bin/test_parser
```

## C# 支持

项目编译为共享库 (`libyini.so`/`yini.dll`)，可通过P/Invoke在C#中使用。

## VSCode 插件

计划开发VSCode插件，提供：
- 语法高亮
- 代码补全
- 实时验证
- 错误提示

## 路线图

- [x] Lexer实现（词法分析）
- [x] Parser实现（语法分析）
- [x] 基础值类型支持
- [x] 继承机制
- [x] 宏定义和引用
- [x] 算术运算
- [x] Schema验证（框架）
- [x] YMETA文件生成和缓存
- [x] CLI工具（完整实现）
- [x] C# P/Invoke绑定
- [x] VSCode插件（语法高亮）

**项目状态**: ✅ **v1.0.0 完成** (2025-10-06)

## 许可证

请查看 LICENSE 文件

## 贡献

欢迎提交Issue和Pull Request！

---

Made with ❤️ for Game Development
