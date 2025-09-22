cmake_minimum_required(VERSION 3.10)
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files for the YINI library components
set(YINI_SOURCES
    src/Lexer/Lexer.cpp
    src/Parser/Parser.cpp
    src/Parser/AST.cpp
    src/Processor/Processor.cpp
    src/YMeta/Serializer.cpp
    src/YMeta/Deserializer.cpp
)

# CLI executable
add_executable(yini_cli src/CLI/main.cpp ${YINI_SOURCES})

# Tests executable
enable_testing()
add_executable(yini_tests tests/test_runner.cpp ${YINI_SOURCES})

# A simple test using the test runner's exit code
add_test(NAME YiniCoreTests COMMAND yini_tests)
