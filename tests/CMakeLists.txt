message(STATUS "--- Configuring tests subdirectory ---")

# Add a test executable
add_executable(yini_tests
    lexer_test.cpp
    parser_test.cpp
    runtime_test.cpp
)

# Link the test executable against GoogleTest and our core library
target_link_libraries(yini_tests
    PRIVATE
    gtest_main
    gmock
    yini_core
)

# Include GoogleTest in this project
include(GoogleTest)
# Discover and add tests to CTest
gtest_discover_tests(yini_tests)
