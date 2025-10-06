# Enable testing for the project
enable_testing()

# Include FetchContent module to fetch dependencies
include(FetchContent)

# Declare GoogleTest as a dependency
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Make googletest available
FetchContent_MakeAvailable(googletest)

# Add subdirectories containing the actual tests
add_subdirectory(Lexer)
add_subdirectory(Parser)