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

# Dyna test
add_executable(dyna_test DynaTest.cpp)
target_link_libraries(dyna_test Yini GTest::gtest_main)
add_test(NAME dyna_test COMMAND dyna_test)

# Serialization test
add_executable(serialization_test SerializationTest.cpp)
target_link_libraries(serialization_test Yini GTest::gtest_main)
add_test(NAME serialization_test COMMAND serialization_test)

# CLI test
add_executable(cli_test CliTest.cpp)
target_link_libraries(cli_test Yini GTest::gtest_main)
add_dependencies(cli_test yini-cli)

# Pass the path to the yini-cli executable to the test
target_compile_definitions(cli_test PRIVATE YINI_CLI_PATH="$<TARGET_FILE:yini-cli>")

add_test(NAME cli_test COMMAND cli_test)

# Syntax Error test
add_executable(syntax_error_test SyntaxErrorTest.cpp)
target_link_libraries(syntax_error_test Yini GTest::gtest_main)
add_test(NAME syntax_error_test COMMAND syntax_error_test)

# Complex Interpreter test
add_executable(complex_interpreter_test ComplexInterpreterTest.cpp)
target_link_libraries(complex_interpreter_test Yini GTest::gtest_main)
add_test(NAME complex_interpreter_test COMMAND complex_interpreter_test)

# C-API test
add_executable(c_api_test CApiTest.cpp)
target_link_libraries(c_api_test Yini GTest::gtest_main)
add_test(NAME c_api_test COMMAND c_api_test)

# Cross-reference test
add_executable(xref_test XRefTest.cpp)
target_link_libraries(xref_test Yini GTest::gtest_main)
add_test(NAME xref_test COMMAND xref_test)