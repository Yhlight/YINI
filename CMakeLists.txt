cmake_minimum_required(VERSION 3.14)
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

FetchContent_Declare(
  lz4
  GIT_REPOSITORY https://github.com/lz4/lz4.git
  GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(lz4)
add_subdirectory(${lz4_SOURCE_DIR}/build/cmake ${lz4_BINARY_DIR})
set_property(TARGET lz4_static PROPERTY POSITION_INDEPENDENT_CODE ON)

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
