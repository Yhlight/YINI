# Add a single test executable
add_executable(yini_tests
    yini_tests.cpp
)

# Link against GTest and the libraries to be tested
target_link_libraries(yini_tests PRIVATE
    gtest_main
    yini_parser
    yini_lexer
)

# Include the project's public headers
target_include_directories(yini_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Register with CTest
include(GoogleTest)
gtest_discover_tests(yini_tests)