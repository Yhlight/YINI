# Define the core YINI library
# This will be a shared library to be used by C# P/Invoke
add_library(yini_core SHARED
    Lexer/Lexer.cpp
    Parser/Parser.cpp
    Runtime/Runtime.cpp
    c_api.cpp
)

# Public headers for the library
target_include_directories(yini_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}" # For internal includes
    "${CMAKE_CURRENT_SOURCE_DIR}/../include" # For public C API
)

if(YINI_BUILD_CLI)
    add_subdirectory(CLI)
endif()
