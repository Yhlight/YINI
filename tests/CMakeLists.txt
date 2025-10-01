# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Lexer test
add_executable(lexer_test LexerTest.cpp)
target_link_libraries(lexer_test Yini GTest::gtest_main)
add_test(NAME lexer_test COMMAND lexer_test)

# Parser test
add_executable(parser_test ParserTest.cpp)
target_link_libraries(parser_test Yini GTest::gtest_main)
add_test(NAME parser_test COMMAND parser_test)

# Expression test
add_executable(expression_test ExpressionTest.cpp)
target_link_libraries(expression_test Yini GTest::gtest_main)
add_test(NAME expression_test COMMAND expression_test)