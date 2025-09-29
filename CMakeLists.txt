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

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_subdirectory(fuzz)
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
target_link_libraries(yini-lsp PRIVATE yini nlohmann_json::nlohmann_json)
install(TARGETS yini-lsp DESTINATION bin)

# --- CPack Configuration ---
include(CPack)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "YINI Configuration File Library and Tools")
set(CPACK_SOURCE_GENERATOR "TGZ")