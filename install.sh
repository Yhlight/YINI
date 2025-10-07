#!/bin/bash
# YINI v2.5 Installation Script
# Usage: sudo ./install.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

echo "╔════════════════════════════════════════════════════╗"
echo "║       YINI v2.5 Installation Script                ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ] && [ -z "$INSTALL_PREFIX" ]; then 
    echo "⚠️  Note: Installing to /usr/local requires sudo"
    echo "   Run: sudo ./install.sh"
    echo "   Or set INSTALL_PREFIX: INSTALL_PREFIX=~/.local ./install.sh"
    echo ""
fi

echo "Installation Configuration:"
echo "  Source: $SCRIPT_DIR"
echo "  Build: $BUILD_DIR"
echo "  Install prefix: $INSTALL_PREFIX"
echo ""

# Step 1: Build
echo "Step 1/4: Building YINI..."
cd "$SCRIPT_DIR"
python3 build.py --clean --test

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

# Step 2: Install binaries
echo "Step 2/4: Installing binaries..."
mkdir -p "$INSTALL_PREFIX/bin"
cp "$BUILD_DIR/bin/yini_cli" "$INSTALL_PREFIX/bin/"
cp "$BUILD_DIR/bin/yini_lsp" "$INSTALL_PREFIX/bin/"
chmod +x "$INSTALL_PREFIX/bin/yini_cli"
chmod +x "$INSTALL_PREFIX/bin/yini_lsp"

echo "  ✅ Installed yini_cli to $INSTALL_PREFIX/bin/"
echo "  ✅ Installed yini_lsp to $INSTALL_PREFIX/bin/"
echo ""

# Step 3: Install libraries
echo "Step 3/4: Installing libraries..."
mkdir -p "$INSTALL_PREFIX/lib"
cp "$BUILD_DIR/lib/libyini.so" "$INSTALL_PREFIX/lib/"
cp "$BUILD_DIR/lib/libyini_lexer.a" "$INSTALL_PREFIX/lib/"
cp "$BUILD_DIR/lib/libyini_parser.a" "$INSTALL_PREFIX/lib/"

echo "  ✅ Installed libraries to $INSTALL_PREFIX/lib/"
echo ""

# Step 4: Install headers
echo "Step 4/4: Installing headers..."
mkdir -p "$INSTALL_PREFIX/include/yini"
cp -r "$SCRIPT_DIR/include/"* "$INSTALL_PREFIX/include/yini/"

echo "  ✅ Installed headers to $INSTALL_PREFIX/include/yini/"
echo ""

# Update library cache
if command -v ldconfig &> /dev/null && [ "$EUID" -eq 0 ]; then
    ldconfig
    echo "  ✅ Updated library cache"
fi

echo "╔════════════════════════════════════════════════════╗"
echo "║          Installation Complete! ✅                  ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""
echo "Installed components:"
echo "  • yini_cli  - CLI tool"
echo "  • yini_lsp  - LSP server"
echo "  • libyini   - Shared library"
echo "  • Headers   - Development files"
echo ""
echo "Quick start:"
echo "  1. Test CLI:"
echo "     yini_cli"
echo ""
echo "  2. Test LSP:"
echo "     which yini_lsp"
echo ""
echo "  3. Install VSCode extension:"
echo "     cd $SCRIPT_DIR/vscode-plugin"
echo "     npm install"
echo ""
echo "  4. Configure VSCode:"
echo "     Settings → yini.lsp.path → \"yini_lsp\""
echo ""
echo "For more info: cat $SCRIPT_DIR/README.md"
echo ""
