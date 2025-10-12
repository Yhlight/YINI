cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0135 NEW) # Handle FetchContent timestamp behavior
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# Add LZ4 using FetchContent
FetchContent_Declare(
  lz4
  GIT_REPOSITORY https://github.com/lz4/lz4.git
  GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(lz4)
add_subdirectory(${lz4_SOURCE_DIR}/build/cmake ${CMAKE_BINARY_DIR}/lz4-build)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.10.5
)
FetchContent_MakeAvailable(nlohmann_json)
set(NLOHMANN_JSON_INCLUDE_DIR ${nlohmann_json_SOURCE_DIR}/include CACHE INTERNAL "nlohmann_json include directory")


add_subdirectory(src/Lexer)
add_subdirectory(src/Parser)
add_subdirectory(src/Resolver)
add_subdirectory(src/Ymeta)
add_subdirectory(src/Validator)
add_subdirectory(src/Interop)
add_subdirectory(src/CLI)

enable_testing()
add_subdirectory(tests)
