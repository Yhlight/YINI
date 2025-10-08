cmake_minimum_required(VERSION 3.10)
project(YINI)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for the different components of the project
add_subdirectory(src/Lexer)
add_subdirectory(src/Parser)
add_subdirectory(src/CLI)

# Add test directory
enable_testing()
add_subdirectory(tests)