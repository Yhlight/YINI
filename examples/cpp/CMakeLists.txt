cmake_minimum_required(VERSION 3.15)
project(YiniExample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the YINI library
# This assumes YINI has been installed, so find_package can locate it.
# Users might need to set CMAKE_PREFIX_PATH for this to work in their environment.
find_package(Yini REQUIRED)

add_executable(yini-example main.cpp)

# Link the Yini::Yini library target, which also handles include directories.
target_link_libraries(yini-example PRIVATE Yini::Yini)

# --- Optional: Installation ---
# This shows how to install the example executable.
install(TARGETS yini-example
    RUNTIME DESTINATION bin
)