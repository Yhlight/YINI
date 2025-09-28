# Add test executable
add_executable(
  test_runner
  test_lexer.cpp
  test_parser.cpp
)

# Link against gtest and gmock
target_link_libraries(
  test_runner
  GTest::gtest_main
  yini
)

# Include GTest
include(GoogleTest)

# Discover and add tests to CTest
gtest_discover_tests(test_runner)