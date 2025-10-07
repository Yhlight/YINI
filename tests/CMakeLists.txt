# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(yini_tests)

# Link the test executable against the YINI library and Google Test
target_link_libraries(yini_tests PRIVATE yini GTest::gtest_main)

# Add the test to CTest
add_test(NAME yini_tests COMMAND yini_tests)

# Add source files for the tests
target_sources(yini_tests PRIVATE
  test_lexer.cpp
  test_parser.cpp
  test_resolver.cpp
  test_cli.cpp
)

# Pass the location of the yini_cli executable to the tests
target_compile_definitions(yini_tests PRIVATE YINI_CLI_EXECUTABLE="$<TARGET_FILE:yini_cli>")