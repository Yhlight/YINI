cmake_minimum_required(VERSION 3.10)
project(YINI)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the 'src' directory to the build
add_subdirectory(src)

# Enable testing with CTest
enable_testing()

# Add the 'tests' directory
add_subdirectory(tests)