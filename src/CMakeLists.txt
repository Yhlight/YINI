# Define the core library as a shared library for P/Invoke
add_library(yini_core SHARED)

# Add sources from subdirectories
target_sources(yini_core
    PRIVATE
        Lexer/Lexer.cpp
        Interop.cpp
        Parser/Parser.cpp
)

# Public headers
target_include_directories(yini_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Explicitly set the output directory for the shared library
set_target_properties(yini_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Add subdirectories for more complex logic
# add_subdirectory(Lexer)
# add_subdirectory(Parser)
# add_subdirectory(CLI)