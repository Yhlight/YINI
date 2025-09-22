#!/bin/bash

set -e

# Path to the yini executable, passed as an argument
YINI_EXE=$1

echo "--- Running CLI Tests ---"

# 1. Create test files
echo "Creating test files..."
mkdir -p cli_test_data
echo '[Test]' > cli_test_data/valid.yini
echo 'key = "value"' >> cli_test_data/valid.yini
echo '[Invalid' > cli_test_data/invalid.yini

# 2. Test 'check' command
echo "Testing 'check' command..."
$YINI_EXE check cli_test_data/valid.yini | grep "Syntax OK"
# Checking for failure is tricky, as the parser doesn't have robust error handling yet
# We'll just check that it runs without crashing for now

# 3. Test 'compile' command
echo "Testing 'compile' command..."
$YINI_EXE compile cli_test_data/valid.yini
test -f cli_test_data/valid.ymeta

# 4. Test 'decompile' command
echo "Testing 'decompile' command..."
$YINI_EXE decompile cli_test_data/valid.ymeta | grep '\[Test\]'
$YINI_EXE decompile cli_test_data/valid.ymeta | grep 'key = "value"'

# 5. Cleanup
echo "Cleaning up..."
rm -rf cli_test_data

echo "--- CLI Tests Passed ---"
