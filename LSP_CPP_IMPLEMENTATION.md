# YINI LSP服务器 - C++原生实现方案

**技术栈**: C++17  
**协议**: Language Server Protocol 3.17  
**状态**: 规划阶段

---

## 🎯 设计目标

1. **纯C++实现** - 无需Node.js/TypeScript
2. **技术栈统一** - 复用现有Parser代码
3. **高性能** - 原生性能，低延迟
4. **轻量级** - 最小化依赖

---

## 🏗️ 架构设计

### 系统架构

```
┌─────────────────┐
│   VSCode        │
│   (最小扩展)     │
└────────┬────────┘
         │ stdio/JSON-RPC
         │
┌────────▼─────────────────────┐
│   yini_lsp (C++17)           │
│                              │
│  ┌────────────────────────┐  │
│  │  JSON-RPC Handler      │  │
│  │  (stdio通信)           │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  LSP Protocol Handler  │  │
│  │  - initialize          │  │
│  │  - textDocument/*      │  │
│  │  - workspace/*         │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  Document Manager      │  │
│  │  (文档缓存)            │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  YINI Parser           │◄─┼─ 复用现有代码
│  │  (已有实现)            │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  Symbol Table          │  │
│  │  (符号索引)            │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  Completion Engine     │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │  Diagnostics Engine    │  │
│  └────────────────────────┘  │
└──────────────────────────────┘
```

---

## 📂 目录结构

```
/workspace/
├── src/
│   ├── LSP/                    # 新增LSP服务器
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp           # LSP服务器入口
│   │   ├── JSONRPCHandler.cpp # JSON-RPC处理
│   │   ├── LSPServer.cpp      # LSP协议实现
│   │   ├── DocumentManager.cpp # 文档管理
│   │   ├── SymbolTable.cpp    # 符号表
│   │   ├── CompletionProvider.cpp  # 补全
│   │   └── DiagnosticsProvider.cpp # 诊断
│   ├── Lexer/                 # 已有
│   ├── Parser/                # 已有
│   └── CLI/                   # 已有
├── include/
│   └── LSP/
│       ├── JSONRPCHandler.h
│       ├── LSPServer.h
│       └── ...
└── vscode-plugin/             # 最小化扩展
    ├── package.json           # 仅启动yini_lsp
    └── extension.js           # 最小化JS代码
```

---

## 🔧 核心组件实现

### 1. JSON-RPC Handler

**功能**: 处理stdin/stdout的JSON-RPC消息

```cpp
// include/LSP/JSONRPCHandler.h
#ifndef YINI_JSONRPC_HANDLER_H
#define YINI_JSONRPC_HANDLER_H

#include <string>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>  // 使用nlohmann/json库

namespace yini::lsp
{

using json = nlohmann::json;
using MessageHandler = std::function<json(const json&)>;

class JSONRPCHandler
{
public:
    JSONRPCHandler();
    
    // 注册方法处理器
    void registerMethod(const std::string& method, MessageHandler handler);
    
    // 处理输入消息
    void processMessage(const std::string& message);
    
    // 发送响应
    void sendResponse(const json& response);
    
    // 发送通知
    void sendNotification(const std::string& method, const json& params);
    
private:
    std::map<std::string, MessageHandler> methodHandlers;
    
    json handleRequest(const json& request);
    void writeMessage(const json& message);
};

} // namespace yini::lsp

#endif
```

**实现示例**:
```cpp
// src/LSP/JSONRPCHandler.cpp
#include "LSP/JSONRPCHandler.h"
#include <iostream>
#include <sstream>

namespace yini::lsp
{

void JSONRPCHandler::processMessage(const std::string& message)
{
    try
    {
        json request = json::parse(message);
        json response = handleRequest(request);
        sendResponse(response);
    }
    catch (const std::exception& e)
    {
        // 错误处理
        json error_response = {
            {"jsonrpc", "2.0"},
            {"error", {
                {"code", -32700},
                {"message", "Parse error"}
            }}
        };
        sendResponse(error_response);
    }
}

void JSONRPCHandler::writeMessage(const json& message)
{
    std::string content = message.dump();
    std::cout << "Content-Length: " << content.length() << "\r\n\r\n";
    std::cout << content << std::flush;
}

} // namespace yini::lsp
```

### 2. LSP Server

**功能**: 实现LSP协议的各种请求

```cpp
// include/LSP/LSPServer.h
#ifndef YINI_LSP_SERVER_H
#define YINI_LSP_SERVER_H

#include "LSP/JSONRPCHandler.h"
#include "LSP/DocumentManager.h"
#include "Parser.h"
#include <memory>

namespace yini::lsp
{

class LSPServer
{
public:
    LSPServer();
    
    void start();
    
private:
    JSONRPCHandler rpcHandler;
    DocumentManager documentManager;
    
    // LSP方法处理器
    json handleInitialize(const json& params);
    json handleShutdown(const json& params);
    json handleTextDocumentDidOpen(const json& params);
    json handleTextDocumentDidChange(const json& params);
    json handleTextDocumentCompletion(const json& params);
    json handleTextDocumentHover(const json& params);
    json handleTextDocumentDefinition(const json& params);
    
    // 辅助方法
    void publishDiagnostics(const std::string& uri);
};

} // namespace yini::lsp

#endif
```

### 3. Document Manager

**功能**: 管理打开的文档和缓存的AST

```cpp
// include/LSP/DocumentManager.h
#ifndef YINI_DOCUMENT_MANAGER_H
#define YINI_DOCUMENT_MANAGER_H

#include "Parser.h"
#include <string>
#include <map>
#include <memory>

namespace yini::lsp
{

struct Document
{
    std::string uri;
    std::string content;
    int version;
    std::unique_ptr<yini::Parser> parser;
    bool parsed;
};

class DocumentManager
{
public:
    void openDocument(const std::string& uri, 
                      const std::string& content, 
                      int version);
    
    void updateDocument(const std::string& uri, 
                        const std::string& content, 
                        int version);
    
    void closeDocument(const std::string& uri);
    
    Document* getDocument(const std::string& uri);
    
    // 获取解析后的Parser
    yini::Parser* getParser(const std::string& uri);
    
private:
    std::map<std::string, std::unique_ptr<Document>> documents;
    
    void parseDocument(Document* doc);
};

} // namespace yini::lsp

#endif
```

### 4. Completion Provider

**功能**: 提供自动补全

```cpp
// include/LSP/CompletionProvider.h
#ifndef YINI_COMPLETION_PROVIDER_H
#define YINI_COMPLETION_PROVIDER_H

#include "Parser.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

struct Position
{
    int line;
    int character;
};

struct CompletionItem
{
    std::string label;
    int kind;  // LSP CompletionItemKind
    std::string detail;
    std::string documentation;
};

class CompletionProvider
{
public:
    std::vector<CompletionItem> getCompletions(
        yini::Parser* parser,
        const std::string& content,
        Position position
    );
    
private:
    // 补全关键字
    std::vector<CompletionItem> completeKeywords(const std::string& prefix);
    
    // 补全宏引用
    std::vector<CompletionItem> completeMacros(
        yini::Parser* parser, 
        const std::string& prefix
    );
    
    // 补全横截面引用
    std::vector<CompletionItem> completeCrossSectionRefs(
        yini::Parser* parser,
        const std::string& prefix
    );
};

} // namespace yini::lsp

#endif
```

---

## 📦 依赖管理

### 必需依赖

1. **nlohmann/json** - JSON解析库
   ```cmake
   # 使用header-only库，无需编译
   include(FetchContent)
   FetchContent_Declare(
     json
     URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
   )
   FetchContent_MakeAvailable(json)
   ```

### 可选依赖

- 无其他依赖，纯C++17标准库

---

## 🛠️ CMake配置

```cmake
# src/LSP/CMakeLists.txt
add_executable(yini_lsp
    main.cpp
    JSONRPCHandler.cpp
    LSPServer.cpp
    DocumentManager.cpp
    CompletionProvider.cpp
    DiagnosticsProvider.cpp
    SymbolTable.cpp
)

target_include_directories(yini_lsp PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(yini_lsp PRIVATE
    yini_parser
    yini_lexer
    nlohmann_json::nlohmann_json
)

# 安装LSP服务器
install(TARGETS yini_lsp DESTINATION bin)
```

---

## 📝 LSP协议实现

### 支持的功能

#### Phase 1 - 基础功能
- [x] `initialize` - 初始化
- [x] `shutdown` - 关闭
- [x] `textDocument/didOpen` - 打开文档
- [x] `textDocument/didChange` - 文档变更
- [x] `textDocument/publishDiagnostics` - 发布诊断

#### Phase 2 - 智能功能
- [ ] `textDocument/completion` - 自动补全
- [ ] `textDocument/hover` - 悬停提示
- [ ] `textDocument/definition` - 定义跳转

#### Phase 3 - 高级功能
- [ ] `textDocument/formatting` - 格式化
- [ ] `textDocument/documentSymbol` - 符号
- [ ] `textDocument/references` - 引用查找

---

## 🎨 VSCode扩展（最小化）

```javascript
// vscode-plugin/extension.js
const vscode = require('vscode');
const { LanguageClient } = require('vscode-languageclient/node');
const path = require('path');

let client;

function activate(context) {
    // LSP服务器路径
    const serverCommand = path.join(
        context.extensionPath, 
        '..', 'build', 'bin', 'yini_lsp'
    );
    
    // 服务器选项
    const serverOptions = {
        command: serverCommand,
        args: []
    };
    
    // 客户端选项
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'yini' }]
    };
    
    // 创建LSP客户端
    client = new LanguageClient(
        'yiniLanguageServer',
        'YINI Language Server',
        serverOptions,
        clientOptions
    );
    
    // 启动客户端
    client.start();
}

function deactivate() {
    if (client) {
        return client.stop();
    }
}

module.exports = { activate, deactivate };
```

```json
// vscode-plugin/package.json
{
  "name": "yini-language-support",
  "version": "2.0.0",
  "engines": {
    "vscode": "^1.60.0"
  },
  "activationEvents": [
    "onLanguage:yini"
  ],
  "main": "./extension.js",
  "contributes": {
    "languages": [
      {
        "id": "yini",
        "extensions": [".yini", ".YINI"],
        "configuration": "./language-configuration.json"
      }
    ],
    "grammars": [
      {
        "language": "yini",
        "scopeName": "source.yini",
        "path": "./syntaxes/yini.tmLanguage.json"
      }
    ]
  },
  "dependencies": {
    "vscode-languageclient": "^8.1.0"
  }
}
```

---

## 🚀 实施步骤

### Step 1: 搭建基础框架（1周）
```bash
# 创建LSP目录结构
mkdir -p src/LSP include/LSP

# 添加nlohmann/json依赖到CMake

# 实现JSONRPCHandler基础类
```

### Step 2: 实现基础协议（1周）
- initialize/shutdown
- textDocument/didOpen
- textDocument/didChange
- 文档管理器

### Step 3: 实现诊断功能（1周）
- 集成Parser进行语法检查
- publishDiagnostics实现
- 错误位置映射

### Step 4: 实现补全功能（1-2周）
- 关键字补全
- 宏引用补全
- 横截面引用补全

### Step 5: 测试和优化（1周）
- 性能测试
- 内存泄漏检查
- 与VSCode集成测试

---

## 💡 实现亮点

### 1. 零运行时依赖
- 编译为单一可执行文件
- 无需Node.js环境
- 启动速度快

### 2. 高性能
- C++原生性能
- 直接复用Parser（无IPC开销）
- 智能缓存策略

### 3. 技术栈统一
- 全部C++17
- 复用现有代码
- 统一的构建系统

### 4. 轻量级
- VSCode扩展极简（<50行JS）
- 服务器二进制 <5MB
- 运行时内存 <50MB

---

## 📊 性能目标

| 指标 | 目标 |
|------|------|
| 启动时间 | <100ms |
| 诊断延迟 | <50ms |
| 补全延迟 | <30ms |
| 内存占用 | <50MB |
| 二进制大小 | <5MB |

---

## ✅ 成功标准

### 功能完整性
- [x] 实时语法错误检测
- [x] 关键字自动补全
- [x] 宏引用补全 (`@name`)
- [x] 横截面引用补全 (`@{Section.key}`)
- [x] 悬停显示值信息
- [x] 定义跳转

### 用户体验
- [x] 安装简单（VSCode插件市场一键安装）
- [x] 零配置启动
- [x] 响应迅速（<50ms）
- [x] 错误提示清晰

---

## 📚 参考实现

### C++ LSP实现参考
- **clangd** - LLVM的C++ LSP服务器
- **rust-analyzer** - Rust LSP服务器（可参考架构）
- **vscode-cpptools** - Microsoft的C++ LSP实现

### JSON-RPC库
- **nlohmann/json** - 现代C++ JSON库
- 手动实现stdin/stdout通信（简单）

---

## 🎯 下一步行动

1. **添加nlohmann/json依赖**
2. **创建LSP目录结构**
3. **实现JSONRPCHandler基础类**
4. **实现initialize协议**
5. **测试基础通信**

---

**优势总结**:
- ✅ 纯C++，技术栈统一
- ✅ 高性能，低延迟
- ✅ 零依赖，易部署
- ✅ 复用现有代码，开发效率高

---

**实施计划**: 6周完成v1.3.0  
**状态**: ✅ 架构设计完成，准备开始编码  
**下一步**: 创建LSP基础框架

---

**规划日期**: 2025-10-06  
**技术栈**: C++17 + nlohmann/json  
**项目**: YINI Language Server (C++ Native)
