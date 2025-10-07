#!/bin/bash
# Build script for YINI C# bindings
# This script can be run once Mono or .NET SDK is installed

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
LIB_DIR="$BUILD_DIR/lib"

echo "=== Building YINI C# Example ==="
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"

# Check if Mono or .NET SDK is available
if command -v mcs &> /dev/null; then
    echo "Using Mono C# compiler (mcs)"
    COMPILER="mcs"
elif command -v csc &> /dev/null; then
    echo "Using Microsoft C# compiler (csc)"
    COMPILER="csc"
elif command -v dotnet &> /dev/null; then
    echo "Using .NET SDK"
    COMPILER="dotnet"
else
    echo "Error: No C# compiler found!"
    echo "Please install Mono or .NET SDK:"
    echo "  Ubuntu/Debian: sudo apt-get install mono-complete"
    echo "  Or download .NET SDK from: https://dotnet.microsoft.com/download"
    exit 1
fi

# Ensure YINI library is built
if [ ! -f "$LIB_DIR/libyini.so" ]; then
    echo "Error: libyini.so not found. Please build the YINI project first."
    echo "Run: ./build.py --clean"
    exit 1
fi

# Build C# bindings
cd "$SCRIPT_DIR"

if [ "$COMPILER" = "mcs" ]; then
    echo "Compiling YINI.cs..."
    mcs -target:library -out:YINI.dll YINI.cs
    
    echo "Compiling Example.cs..."
    mcs -reference:YINI.dll -out:Example.exe Example.cs
    
    echo ""
    echo "=== Build Successful! ==="
    echo ""
    echo "To run the example:"
    echo "  cd $SCRIPT_DIR"
    echo "  export LD_LIBRARY_PATH=$LIB_DIR:\$LD_LIBRARY_PATH"
    echo "  mono Example.exe"
    
elif [ "$COMPILER" = "dotnet" ]; then
    echo "Using .NET SDK build..."
    # Create a simple project file if needed
    echo "Note: For .NET SDK, please create a .csproj file"
    echo "See bindings/csharp/README.md for instructions"
fi

echo ""
echo "Library path: $LIB_DIR/libyini.so"
