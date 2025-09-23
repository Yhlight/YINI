# CMake build system for the YINI project

# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name and language
project(YINI CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Shared Library Target (for P/Invoke) ---

# Gather all source files for the library
set(YINI_LIB_SOURCES
    src/Lexer/Lexer.cpp
    src/Parser/Parser.cpp
    src/Parser/YiniLoader.cpp
    src/Parser/Ymeta.cpp
    src/Interop/YiniInterop.cpp
    src/Json/Json.cpp
)

# Add the shared library target
add_library(yini SHARED ${YINI_LIB_SOURCES})

# Define YINI_EXPORTS so that YINI_API expands to dllexport on Windows
target_compile_definitions(yini PRIVATE YINI_EXPORTS)

# Add the 'src' directory to the include path
target_include_directories(yini PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- CLI Executable Target (for testing/standalone use) ---

set(YINI_CLI_SOURCES
    src/main.cpp
    src/CLI/CLI.cpp
)

add_executable(yini-cli ${YINI_CLI_SOURCES})

# Link the CLI against the YINI library
target_link_libraries(yini-cli PRIVATE yini)


# --- Common Settings ---

# Optional: Add compiler warnings for better code quality
if(MSVC)
    target_compile_options(yini PRIVATE /W4)
    target_compile_options(yini-cli PRIVATE /W4)
else()
    target_compile_options(yini PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(yini-cli PRIVATE -Wall -Wextra -Wpedantic)
endif()
