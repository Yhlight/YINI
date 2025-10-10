cmake_minimum_required(VERSION 3.10)
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src/Lexer)
add_subdirectory(src/Parser)
add_subdirectory(src/Resolver)
add_subdirectory(src/CLI)

enable_testing()
add_subdirectory(tests)