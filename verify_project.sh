#!/bin/bash
# YINI 项目验证脚本
# 用于快速验证项目的构建和测试状态

set -e

echo "╔══════════════════════════════════════════════════════╗"
echo "║     YINI 项目验证脚本                                ║"
echo "║     Version 2.5.0                                    ║"
echo "╚══════════════════════════════════════════════════════╝"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查函数
check_step() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $1"
    else
        echo -e "${RED}✗${NC} $1"
        exit 1
    fi
}

# 1. 检查项目结构
echo "1. 检查项目结构..."
test -d src/Lexer && check_step "  Lexer 源代码目录存在"
test -d src/Parser && check_step "  Parser 源代码目录存在"
test -d src/CLI && check_step "  CLI 源代码目录存在"
test -d src/LSP && check_step "  LSP 源代码目录存在"
test -d include && check_step "  include 目录存在"
test -d tests && check_step "  tests 目录存在"
test -d examples && check_step "  examples 目录存在"
test -d bindings/csharp && check_step "  C# bindings 目录存在"
test -d vscode-plugin && check_step "  VSCode 插件目录存在"
echo ""

# 2. 检查关键文件
echo "2. 检查关键文件..."
test -f CMakeLists.txt && check_step "  CMakeLists.txt 存在"
test -f build.py && check_step "  build.py 存在"
test -f .gitignore && check_step "  .gitignore 存在"
test -f YINI.md && check_step "  YINI.md (语言规范) 存在"
test -f README.md && check_step "  README.md 存在"
echo ""

# 3. 清理并构建项目
echo "3. 构建项目 (Release模式)..."
python3 build.py --clean --build-type Release > /tmp/yini_build.log 2>&1
check_step "  项目构建成功"
echo ""

# 4. 检查构建产物
echo "4. 检查构建产物..."
test -f build/lib/libyini.so && check_step "  共享库 libyini.so 生成"
test -f build/lib/libyini_lexer.a && check_step "  静态库 libyini_lexer.a 生成"
test -f build/lib/libyini_parser.a && check_step "  静态库 libyini_parser.a 生成"
test -f build/bin/yini_cli && check_step "  CLI 工具 yini_cli 生成"
test -f build/bin/yini_lsp && check_step "  LSP 服务器 yini_lsp 生成"
test -f build/bin/test_lexer && check_step "  Lexer 测试程序生成"
test -f build/bin/test_parser && check_step "  Parser 测试程序生成"
echo ""

# 5. 运行测试
echo "5. 运行测试..."
./build/bin/test_lexer > /tmp/yini_lexer_test.log 2>&1
check_step "  Lexer 测试通过"

./build/bin/test_parser > /tmp/yini_parser_test.log 2>&1
check_step "  Parser 测试通过"
echo ""

# 6. 运行CTest
echo "6. 运行 CTest..."
cd build && ctest --output-on-failure > /tmp/yini_ctest.log 2>&1
cd ..
check_step "  CTest 测试通过"
echo ""

# 7. 检查文档
echo "7. 检查文档..."
test -f docs/IMPLEMENTATION_SUMMARY.md && check_step "  实现总结文档存在"
test -f docs/PROJECT_COMPLETION_REPORT.md && check_step "  项目完成报告存在"
test -f bindings/csharp/README.md && check_step "  C# 绑定文档存在"
test -f vscode-plugin/README.md && check_step "  VSCode 插件文档存在"
echo ""

# 8. 项目统计
echo "8. 项目统计..."
echo "  源文件数量:"
echo "    C++ 头文件: $(find include -name "*.h" | wc -l)"
echo "    C++ 源文件: $(find src -name "*.cpp" | wc -l)"
echo "    测试文件: $(find tests -name "*.cpp" | wc -l)"
echo "    示例文件: $(find examples -name "*.yini" | wc -l)"
echo ""

# 9. 显示版本信息
echo "9. 版本信息..."
grep "project(YINI VERSION" CMakeLists.txt | sed 's/.*VERSION \([0-9.]*\).*/  项目版本: \1/'
echo ""

# 10. 总结
echo "╔══════════════════════════════════════════════════════╗"
echo "║     验证完成！                                       ║"
echo "╚══════════════════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}所有检查都已通过！✓${NC}"
echo ""
echo "后续步骤:"
echo "  1. 查看实现报告: cat IMPLEMENTATION_FIX_REPORT.md"
echo "  2. 测试CLI工具: ./build/bin/yini_cli"
echo "  3. 查看示例文件: ls examples/"
echo "  4. 构建C#绑定: cd bindings/csharp && ./build_csharp.sh"
echo "  5. 安装VSCode插件: 查看 vscode-plugin/README.md"
echo ""
echo "日志文件位于:"
echo "  构建日志: /tmp/yini_build.log"
echo "  测试日志: /tmp/yini_lexer_test.log, /tmp/yini_parser_test.log"
echo "  CTest日志: /tmp/yini_ctest.log"
echo ""
