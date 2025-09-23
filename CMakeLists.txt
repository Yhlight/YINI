cmake_minimum_required(VERSION 3.15)

project(YINI LANGUAGES CXX VERSION 1.0)

# --- Dependencies ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the src directory, which contains the main logic
add_subdirectory(src)

# --- Installation ---
# This defines how to install the library and headers.
install(TARGETS yini_core yini_cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES include/yini.h DESTINATION include)

# --- Testing ---
enable_testing()
add_subdirectory(tests)
