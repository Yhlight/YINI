include(FetchContent)

# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(yini_tests test_lexer.cpp test_parser.cpp test_validation.cpp)

# Link the test executable against GoogleTest
target_link_libraries(yini_tests GTest::gtest_main)

# Link against the project libraries
target_link_libraries(yini_tests Lexer Parser)

# Include the source directory for headers
target_include_directories(yini_tests PUBLIC ../src)