#!/bin/bash

# Exit on error
set -e

# The build system will pass the path to the CLI executable as the first argument
CLI_EXECUTABLE=$1
# The test will be run from the build directory, so paths need to be relative to the project root
TEST_DIR="tests/CLI"
SAMPLE_YINI="$TEST_DIR/sample.yini"
YMETA_FILE="$TEST_DIR/sample.ymeta"
RESTORED_YINI="$TEST_DIR/sample.restored.yini"

# A sequence of commands to test the interactive CLI
COMMANDS=$(cat <<EOF
check $SAMPLE_YINI
parse $SAMPLE_YINI
compile $SAMPLE_YINI $YMETA_FILE
decompile $YMETA_FILE $RESTORED_YINI
help
exit
EOF
)

echo "--- Running CLI End-to-End Test ---"
# Pipe the commands to the CLI and capture the output
OUTPUT=$(echo "$COMMANDS" | "$CLI_EXECUTABLE")

# --- Log Output for Debugging ---
echo "--- CLI Output ---"
echo "$OUTPUT"
echo "------------------"

# --- Assertions ---
echo "--- Verifying Test Results ---"

# Check for success messages for each command
echo "$OUTPUT" | grep "✓✓ File is valid!"
echo "$OUTPUT" | grep "✓ Parse successful!"
echo "$OUTPUT" | grep "✓ Compilation successful!"
echo "$OUTPUT" | grep "✓ Decompilation successful!"

# Check that the help command worked
echo "$OUTPUT" | grep "Available commands:"

# Check that the CLI exited gracefully
echo "$OUTPUT" | grep "Goodbye!"

# Check that the output files were created
if [ ! -f "$YMETA_FILE" ]; then
    echo "✗ ERROR: YMETA file was not created."
    exit 1
fi
if [ ! -f "$RESTORED_YINI" ]; then
    echo "✗ ERROR: Restored YINI file was not created."
    exit 1
fi

echo "✓ All checks passed."

# --- Cleanup ---
echo "--- Cleaning up generated files ---"
rm "$YMETA_FILE"
rm "$RESTORED_YINI"

echo "--- CLI Test Passed Successfully! ✓ ---"
exit 0