#!/bin/bash

# build.sh - A simple script to build the YINI project on Linux/macOS.
# This script uses CMake to generate Makefiles and then uses make to compile the project.

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
# The directory where build files will be generated.
BUILD_DIR="build"

# --- Script ---
echo "--- Starting YINI build process ---"

# Create the build directory if it doesn't exist.
echo "[1/4] Creating build directory: ${BUILD_DIR}"
mkdir -p ${BUILD_DIR}

# Navigate into the build directory.
cd ${BUILD_DIR}
echo "[2/4] Changed directory to: $(pwd)"

# Run CMake to configure the project and generate Makefiles.
echo "[3/4] Running CMake to configure the project..."
cmake ..

# Run make to compile the project.
echo "[4/4] Compiling the project with make..."
make

echo "--- Build complete! ---"
echo "The executable can be found at: ${BUILD_DIR}/yini"
