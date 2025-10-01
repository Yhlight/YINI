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

# Interpreter test
add_executable(interpreter_test InterpreterTest.cpp)
target_link_libraries(interpreter_test Yini GTest::gtest_main)
add_test(NAME interpreter_test COMMAND interpreter_test)

# Include test
add_executable(include_test IncludeTest.cpp)
target_link_libraries(include_test Yini GTest::gtest_main)
add_test(NAME include_test COMMAND include_test)

# CLI test
add_executable(cli_test CliTest.cpp)
target_link_libraries(cli_test Yini GTest::gtest_main)

# Pass the path to the yini-cli executable to the test
target_compile_definitions(cli_test PRIVATE YINI_CLI_PATH="$<TARGET_FILE:yini-cli>")

add_test(NAME cli_test COMMAND cli_test)