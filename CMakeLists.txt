# =============================================================================
# Project Setup
# =============================================================================
cmake_minimum_required(VERSION 3.15)
project(YINI LANGUAGES CXX)

# =============================================================================
# Project Configuration
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Allman brace style is a code formatting rule, not a compiler setting.
# We will enforce this manually and with formatters if available.

# =============================================================================
# Project Structure
# =============================================================================
# Add the 'src' directory to the include path so we can use <YINI/...> style includes
include_directories(src)

# We will define our source files here later
set(YINI_LIB_SOURCES
    src/Lexer/Lexer.cpp
    src/Parser/Parser.cpp
    src/Resolver/Resolver.cpp
    src/Evaluator/Evaluator.cpp
    src/c_api.cpp
)

# =============================================================================
# Library Target (for C# Interop)
# =============================================================================
# We will build a shared library that can be called from C#
add_library(yini SHARED ${YINI_LIB_SOURCES})

# Name the library 'yini' on all platforms to simplify P/Invoke
set_target_properties(yini PROPERTIES
    OUTPUT_NAME "yini"
    # This helps with finding the library on Windows
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Add internal include path for source files to find each other
target_include_directories(yini PRIVATE src)

# Make the public C header available to consumers of the library
target_include_directories(yini PUBLIC include)

# =============================================================================
# Message to user
# =============================================================================
message(STATUS "YINI Project Configured")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build as a shared library for P/Invoke")
