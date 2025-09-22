# YINI
现代化的INI，使用于游戏开发

### VSCode 扩展

位于 `vscode-extension/`，提供语法高亮、诊断、补全、命令编译/反编译、内置文档。

本地开发：

1. 进入扩展目录并安装依赖：
   - `npm install`
2. 构建：
   - `npm run build`
3. 使用 VSCode 打开该目录，按 F5 启动扩展开发宿主。

注意：需要先构建 C++ 工程生成 `yini` CLI 并加入 PATH。
