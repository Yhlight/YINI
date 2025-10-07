# YINI v2.5 快速上手指南

**版本**: 2.5.0 Final  
**难度**: ⭐ 简单  
**时间**: 5分钟

---

## 🚀 三步开始使用

### 方法1: 一键启动（推荐）

```bash
./quick_start.sh
```

搞定！脚本会自动完成构建和测试。

### 方法2: 手动步骤

#### 步骤1: 构建项目

```bash
python3 build.py --clean --test
```

#### 步骤2: 测试CLI工具

```bash
./build/bin/yini_cli
```

输入 `help` 查看命令，输入 `exit` 退出。

#### 步骤3: 安装到系统（可选）

```bash
sudo ./install.sh
```

---

## 💡 第一个YINI文件

### 创建文件

创建 `my_config.yini`:

```yini
// 定义常量
[#define]
SCREEN_WIDTH = 1920
SCREEN_HEIGHT = 1080
MAX_PLAYERS = 16

// 图形设置
[Graphics]
width = @SCREEN_WIDTH
height = @SCREEN_HEIGHT
fullscreen = true
vsync = true
quality = "ultra"

// 服务器设置
[Server]
max_players = @MAX_PLAYERS
port = 8080
name = "My Game Server"

// 游戏设置  
[Game]
difficulty = 2
save_interval = 300
auto_save = true
```

### 解析文件

```bash
./build/bin/yini_cli
yini> parse my_config.yini
```

输出会显示解析后的配置结构。

---

## 🎨 使用VSCode IDE功能

### 安装VSCode扩展

#### 步骤1: 安装LSP服务器

```bash
# 系统级安装
sudo ./install.sh

# 或本地使用
export PATH=$PATH:$(pwd)/build/bin
```

#### 步骤2: 配置VSCode

打开VSCode设置 (`Ctrl+,`)，搜索 "yini"，设置：

```json
{
  "yini.lsp.path": "yini_lsp"
}
```

或者使用项目本地路径：

```json
{
  "yini.lsp.path": "/workspace/build/bin/yini_lsp"
}
```

#### 步骤3: 打开.yini文件

创建或打开任意 `.yini` 文件，IDE功能自动激活！

### IDE功能演示

#### 1. 智能补全

输入 `@` 触发宏补全：

```yini
[#define]
WIDTH = 1920

[Graphics]
w = @    ← 输入@，自动显示WIDTH等宏
```

#### 2. 悬停信息

鼠标悬停在 `@WIDTH` 上：

```
Macro: @WIDTH
Type: integer
Value: 1920
Defined in [#define] section
```

#### 3. 定义跳转

光标放在 `@WIDTH` 上，按 `F12`，跳转到定义：

```yini
[#define]
WIDTH = 1920    ← 跳转到这里
```

#### 4. 查找引用

光标放在 `WIDTH` 上，按 `Shift+F12`，显示所有使用：

```
2 references:
  my_config.yini:9  w = @WIDTH
  my_config.yini:15 panel_w = @WIDTH
```

#### 5. 重命名

光标放在 `WIDTH` 上，按 `F2`，输入新名字，所有引用自动更新。

#### 6. 代码格式化

按 `Shift+Alt+F`，自动格式化整个文件。

#### 7. 文档大纲

按 `Ctrl+Shift+O`，查看文件结构，点击任意符号跳转。

---

## 📚 常用示例

### 示例1: 游戏配置

```yini
[#define]
// 分辨率预设
RES_4K_W = 3840
RES_4K_H = 2160
RES_1080P_W = 1920
RES_1080P_H = 1080

[Graphics]
resolution_width = @RES_1080P_W
resolution_height = @RES_1080P_H
fullscreen = true
vsync = true
anti_aliasing = 4
texture_quality = "high"
shadow_quality = "medium"
```

### 示例2: 服务器配置

```yini
[Database]
host = "localhost"
port = 5432
name = "gamedb"
pool_size = 20

[Server]
listen_port = 8080
max_connections = 1000
timeout = 30
db_host = @{Database.host}
db_port = @{Database.port}
```

### 示例3: 继承配置

```yini
[GraphicsBase]
width = 1920
height = 1080
quality = "medium"

[GraphicsUltra] : GraphicsBase
quality = "ultra"
shadows = true
reflections = true

[GraphicsLow] : GraphicsBase
width = 1280
height = 720
quality = "low"
```

---

## 🔧 CLI工具使用

### 交互模式

```bash
./build/bin/yini_cli
```

**可用命令**:
- `help` - 显示帮助
- `parse <file>` - 解析YINI文件
- `compile <input> <output>` - 编译为YMETA
- `decompile <input> <output>` - 反编译YMETA
- `check <file>` - 语法检查
- `exit` - 退出

### 命令行模式

```bash
# 解析文件
./build/bin/yini_cli parse my_config.yini

# 编译为二进制
./build/bin/yini_cli compile config.yini config.ymeta

# 反编译
./build/bin/yini_cli decompile config.ymeta config_decoded.yini

# 语法检查
./build/bin/yini_cli check config.yini
```

---

## 📖 学习资源

### 基础

1. **语言规范**: `YINI.md`
2. **基础示例**: `examples/simple.yini`
3. **中级示例**: `examples/example.yini`

### 进阶

4. **完整示例**: `examples/comprehensive.yini`
5. **引用解析**: `examples/reference_resolution.yini`

### LSP/IDE

6. **LSP文档**: `LSP_SERVER_README.md`
7. **VSCode扩展**: `vscode-plugin/README.md`

---

## ❓ 常见问题

### Q: 如何检查安装是否成功？

```bash
yini_cli --version
yini_lsp --version  # 会启动服务器，Ctrl+C退出
```

### Q: VSCode没有自动补全？

1. 检查LSP服务器路径配置
2. 查看输出面板: View → Output → YINI Language Server
3. 重启VSCode: Ctrl+Shift+P → Reload Window

### Q: 如何卸载？

```bash
sudo rm /usr/local/bin/yini_cli
sudo rm /usr/local/bin/yini_lsp
sudo rm /usr/local/lib/libyini*
sudo rm -rf /usr/local/include/yini
```

### Q: 支持哪些编辑器？

- ✅ VSCode (完整支持)
- ⏳ Vim/Neovim (通过LSP client)
- ⏳ Emacs (通过LSP client)
- ⏳ Sublime Text (通过LSP client)

---

## 🎯 下一步

1. ✅ 完成快速开始
2. 📖 阅读 `YINI.md` 学习完整语法
3. 💻 在VSCode中尝试IDE功能
4. 🎮 在项目中使用YINI配置文件
5. 📢 分享给团队

---

## 🌟 提示和技巧

### 技巧1: 使用宏避免重复

```yini
[#define]
BASE_URL = "https://api.example.com"

[Endpoints]
users = @BASE_URL + "/users"
posts = @BASE_URL + "/posts"
```

### 技巧2: 使用继承组织配置

```yini
[DefaultSettings]
timeout = 30
retry = 3

[Development] : DefaultSettings
debug = true

[Production] : DefaultSettings
debug = false
```

### 技巧3: 使用Schema验证

```yini
[#schema]
Graphics.width = int required default(1920)
Graphics.height = int required default(1080)
Graphics.fullscreen = bool optional default(false)
```

---

## 📞 获取帮助

- **文档**: 查看项目根目录的 `.md` 文件
- **示例**: 查看 `examples/` 目录
- **问题**: 查看 README.md

---

**YINI - 5分钟上手，终身受益！** 🎮✨

**祝您使用愉快！** 🚀
