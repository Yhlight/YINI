# CMake build system for the YINI project

# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name and language
project(YINI CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gather all source files for the project
set(YINI_SOURCES
    src/main.cpp
    src/CLI/CLI.cpp
    src/Lexer/Lexer.cpp
    src/Parser/Parser.cpp
    src/Parser/YiniLoader.cpp
    src/Parser/Ymeta.cpp
)

# Add the executable target
add_executable(yini ${YINI_SOURCES})

# Add the 'src' directory to the include path so headers can be found
target_include_directories(yini PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optional: Add compiler warnings for better code quality
if(MSVC)
    target_compile_options(yini PRIVATE /W4)
else()
    target_compile_options(yini PRIVATE -Wall -Wextra -Wpedantic)
endif()
