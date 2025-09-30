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

option(YINI_BUILD_TESTS "Build the tests for YINI" ON)

if(YINI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

add_executable(yini-cli
    src/CLI/main.cpp
)
target_link_libraries(yini-cli PRIVATE yini)

add_executable(yini-lsp
    src/LSP/main.cpp
)
target_link_libraries(yini-lsp PRIVATE yini nlohmann_json::nlohmann_json)

# Install rules
include(GNUInstallDirs)
install(TARGETS yini-cli yini-lsp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)