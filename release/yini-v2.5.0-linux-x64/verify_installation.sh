#!/bin/bash
# YINI v2.5.0 Installation Verification Script

echo "╔════════════════════════════════════════════════════╗"
echo "║   YINI v2.5.0 Installation Verification            ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

PASS=0
FAIL=0

# Test function
test_item() {
    local name="$1"
    local command="$2"
    
    echo -n "Testing $name... "
    if eval "$command" &> /dev/null; then
        echo "✅ PASS"
        ((PASS++))
    else
        echo "❌ FAIL"
        ((FAIL++))
    fi
}

# File existence tests
echo "📁 File Existence Tests:"
test_item "yini_cli binary" "[ -f bin/yini_cli ]"
test_item "yini_lsp binary" "[ -f bin/yini_lsp ]"
test_item "libyini.so library" "[ -f lib/libyini.so ]"
test_item "Headers" "[ -d include ]"
test_item "Examples" "[ -d examples ]"
echo ""

# Executable tests
echo "🔧 Executable Tests:"
test_item "yini_cli executable" "[ -x bin/yini_cli ]"
test_item "yini_lsp executable" "[ -x bin/yini_lsp ]"
test_item "install.sh executable" "[ -x install.sh ]"
echo ""

# Checksum tests
echo "🔐 Integrity Tests:"
if [ -f SHA256SUMS ]; then
    echo -n "Testing checksums... "
    if sha256sum -c SHA256SUMS &> /dev/null; then
        echo "✅ PASS"
        ((PASS++))
    else
        echo "❌ FAIL"
        ((FAIL++))
    fi
else
    echo "⚠️  SHA256SUMS not found"
fi
echo ""

# Functional tests
echo "⚡ Functional Tests:"
test_item "yini_cli runs" "./bin/yini_cli < /dev/null"
echo ""

# Summary
echo "╔════════════════════════════════════════════════════╗"
echo "║              Verification Summary                   ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""
echo "  Passed: $PASS"
echo "  Failed: $FAIL"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "✅ All tests passed! Package is ready for deployment."
    exit 0
else
    echo "❌ Some tests failed. Please review the package."
    exit 1
fi
