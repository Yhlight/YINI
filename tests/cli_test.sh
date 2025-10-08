#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Build the project
python3 /app/build.py --build

# --- Parse Command Test ---
echo "Testing 'parse' command..."
output_parse=$(/app/build/src/CLI/yini_cli parse /app/tests/test.yini)
expected_parse="File parsed successfully.
[Section1]
key1 = value1
key2 = value2
[Section2]
key3 = value3"

if [ "$output_parse" == "$expected_parse" ]; then
    echo "✔ 'parse' command test passed!"
else
    echo "❌ 'parse' command test failed!"
    echo "Expected output:"
    echo "$expected_parse"
    echo "Actual output:"
    echo "$output_parse"
    false
fi

# --- Check Command Tests ---
echo "Testing 'check' command..."

# Test with a valid file
output_valid=$(/app/build/src/CLI/yini_cli check /app/tests/test.yini)
expected_valid="File syntax is valid."
if [ "$output_valid" == "$expected_valid" ]; then
    echo "✔ 'check' command with valid file passed!"
else
    echo "❌ 'check' command with valid file failed!"
    echo "Expected output: $expected_valid"
    echo "Actual output: $output_valid"
    false
fi

# Test with an invalid file
output_invalid=$(/app/build/src/CLI/yini_cli check /app/tests/invalid.yini 2>&1 || true)
expected_invalid_prefix="File syntax is invalid:"
if [[ "$output_invalid" == "$expected_invalid_prefix"* ]]; then
    echo "✔ 'check' command with invalid file passed!"
else
    echo "❌ 'check' command with invalid file failed!"
    echo "Expected prefix: $expected_invalid_prefix"
    echo "Actual output: $output_invalid"
    false
fi


# --- Compile/Decompile Tests ---
echo "Testing 'compile' and 'decompile' commands..."

# Compile the test file
compile_output=$(/app/build/src/CLI/yini_cli compile /app/tests/test.yini)
expected_compile_output="File compiled successfully to /app/tests/test.ymeta"
if [ "$compile_output" != "$expected_compile_output" ]; then
    echo "❌ 'compile' command failed!"
    echo "Expected: $expected_compile_output"
    echo "Actual:   $compile_output"
    false
fi

# Check if the output file exists
if [ ! -f /app/tests/test.ymeta ]; then
    echo "❌ 'compile' command did not create the output file!"
    false
fi
echo "✔ 'compile' command test passed!"

# Decompile the file and check the output
decompile_output=$(/app/build/src/CLI/yini_cli decompile /app/tests/test.ymeta)
expected_decompile_output="[Section1]
key1 = value1
key2 = value2
[Section2]
key3 = value3"

if [ "$decompile_output" == "$expected_decompile_output" ]; then
    echo "✔ 'decompile' command test passed!"
else
    echo "❌ 'decompile' command test failed!"
    echo "Expected output:"
    echo "$expected_decompile_output"
    echo "Actual output:"
    echo "$decompile_output"
    false
fi

# Clean up the generated file
rm /app/tests/test.ymeta

echo "All CLI tests passed!"