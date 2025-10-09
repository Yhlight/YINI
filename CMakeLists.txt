cmake_minimum_required(VERSION 3.10)
project(YINI)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch nlohmann_json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Add subdirectories for the different components of the project
add_subdirectory(src/Lexer)
add_subdirectory(src/Parser)
add_subdirectory(src/Ymeta)
add_subdirectory(src/Interop)
add_subdirectory(src/LangServer)
add_subdirectory(src/cli)

# Add test directory
enable_testing()
add_subdirectory(tests)