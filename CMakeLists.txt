cmake_minimum_required(VERSION 3.10)
project(YINI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.0
)
set(BENCHMARK_ENABLE_TESTING OFF) # Disable building of benchmark's own tests
FetchContent_MakeAvailable(benchmark)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_subdirectory(fuzz)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(benchmarks)
endif()

add_executable(yini-cli
    src/CLI/main.cpp
)
target_include_directories(yini-cli PRIVATE src)
target_link_libraries(yini-cli PRIVATE yini)
install(TARGETS yini-cli DESTINATION bin)

add_executable(yini-lsp
    src/LSP/main.cpp
)
target_include_directories(yini-lsp PRIVATE src)
target_link_libraries(yini-lsp PRIVATE yini nlohmann_json::nlohmann_json)
install(TARGETS yini-lsp DESTINATION bin)

# Add a post-build command to copy the LSP server to the VS Code extension directory
add_custom_command(
    TARGET yini-lsp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:yini-lsp>
            ${CMAKE_SOURCE_DIR}/vscode-yini/server/yini-lsp
    COMMENT "Copying yini-lsp to VS Code extension directory"
)

# --- CPack Configuration ---
include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "YINI Configuration File Library and Tools")
set(CPACK_SOURCE_GENERATOR "TGZ")