#!/bin/bash
# YINI v2.5 Quick Start Script
# One-command setup for development

set -e

echo "╔════════════════════════════════════════════════════╗"
echo "║       YINI v2.5 Quick Start                        ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Step 1: Build
echo "Step 1/3: Building YINI project..."
python3 build.py --clean --test

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

# Step 2: Test CLI
echo "Step 2/3: Testing CLI tool..."
./build/bin/yini_cli <<EOF
help
exit
EOF

echo "✅ CLI works"
echo ""

# Step 3: Verify LSP
echo "Step 3/3: Verifying LSP server..."
if [ -f "./build/bin/yini_lsp" ]; then
    echo "✅ LSP server built: $(ls -lh ./build/bin/yini_lsp | awk '{print $5}')"
else
    echo "❌ LSP server not found!"
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════╗"
echo "║          Quick Start Complete! ✅                   ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""
echo "Next steps:"
echo ""
echo "1. Install system-wide (optional):"
echo "   sudo ./install.sh"
echo ""
echo "2. Try CLI tool:"
echo "   ./build/bin/yini_cli"
echo ""
echo "3. Parse example file:"
echo "   ./build/bin/yini_cli"
echo "   yini> parse examples/simple.yini"
echo ""
echo "4. Install VSCode extension:"
echo "   cd vscode-plugin"
echo "   npm install"
echo ""
echo "5. Configure VSCode:"
echo "   Add to settings.json:"
echo "   {\"yini.lsp.path\": \"$SCRIPT_DIR/build/bin/yini_lsp\"}"
echo ""
echo "For more info: cat README.md"
echo ""
