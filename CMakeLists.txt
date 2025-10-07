cmake_minimum_required(VERSION 3.10)
project(YINI)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Include subdirectories
add_subdirectory(src)
add_subdirectory(tests)