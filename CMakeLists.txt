cmake_minimum_required(VERSION 3.15)
project(YINI VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Enable testing
enable_testing()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Core library sources
set(CORE_SOURCES
    src/Core/Types.cpp
    src/Core/Value.cpp
)

# Lexer sources
set(LEXER_SOURCES
    src/Lexer/Token.cpp
    src/Lexer/Lexer.cpp
    src/Lexer/LexerState.cpp
)

# Parser sources
set(PARSER_SOURCES
    src/Parser/Parser.cpp
    src/Parser/ParserState.cpp
    src/Parser/ASTNode.cpp
)

# CLI sources
set(CLI_SOURCES
    src/CLI/main.cpp
    src/CLI/CLI.cpp
)

# Create static library
add_library(yini_core STATIC ${CORE_SOURCES})
add_library(yini_lexer STATIC ${LEXER_SOURCES})
add_library(yini_parser STATIC ${PARSER_SOURCES})

# Link libraries
target_link_libraries(yini_lexer yini_core)
target_link_libraries(yini_parser yini_core yini_lexer)

# CLI executable
add_executable(yini ${CLI_SOURCES})
target_link_libraries(yini yini_parser)

# Shared library for P/Invoke
add_library(yini_shared SHARED 
    ${CORE_SOURCES}
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    src/API/CSharpAPI.cpp
)

# Tests
add_subdirectory(tests)

# Install targets
install(TARGETS yini DESTINATION bin)
install(TARGETS yini_shared DESTINATION lib)
