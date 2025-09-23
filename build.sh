#!/bin/bash

# build.sh - A script to build the YINI C++ library and the C# example.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
BUILD_DIR="build"
CSHARP_DIR="csharp_example"

# --- C++ Build ---
echo "--- [1/4] Building C++ Shared Library ---"

# Create the build directory if it doesn't exist.
mkdir -p ${BUILD_DIR}

# Navigate into the build directory.
cd ${BUILD_DIR}

# Run CMake to configure the project and generate Makefiles.
cmake ..

# Run make to compile the project.
make
echo "--- C++ Library build complete! ---"

# --- C# Build & Run ---
# Navigate back to the root directory
cd ..

echo "\n--- [2/4] Preparing C# Example ---"
# Copy the native library to the C# project directory so it can be found at runtime.
cp "${BUILD_DIR}/libyini.so" "${CSHARP_DIR}/"
echo "Copied libyini.so to ${CSHARP_DIR}/"

# Navigate to the C# project directory
cd ${CSHARP_DIR}

echo "\n--- [3/4] Building C# Project ---"
dotnet build

echo "\n--- [4/4] Running C# Example ---"
dotnet run

echo "\n--- Full build and test complete! ---"
