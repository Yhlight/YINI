# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# --- Core Component Tests ---
add_executable(lexer_test LexerTest.cpp)
target_link_libraries(lexer_test Yini GTest::gtest_main)
add_test(NAME lexer_test COMMAND lexer_test)
set_target_properties(lexer_test PROPERTIES FOLDER "Tests/Core")

add_executable(parser_test ParserTest.cpp)
target_link_libraries(parser_test Yini GTest::gtest_main)
add_test(NAME parser_test COMMAND parser_test)
set_target_properties(parser_test PROPERTIES FOLDER "Tests/Core")

add_executable(expression_test ExpressionTest.cpp)
target_link_libraries(expression_test Yini GTest::gtest_main)
add_test(NAME expression_test COMMAND expression_test)
set_target_properties(expression_test PROPERTIES FOLDER "Tests/Core")

add_executable(interpreter_test InterpreterTest.cpp)
target_link_libraries(interpreter_test Yini GTest::gtest_main)
add_test(NAME interpreter_test COMMAND interpreter_test)
set_target_properties(interpreter_test PROPERTIES FOLDER "Tests/Core")

add_executable(c_api_test CApiTest.cpp)
target_link_libraries(c_api_test Yini GTest::gtest_main)
add_test(NAME c_api_test COMMAND c_api_test)
set_target_properties(c_api_test PROPERTIES FOLDER "Tests/Core")

add_executable(syntax_error_test SyntaxErrorTest.cpp)
target_link_libraries(syntax_error_test Yini GTest::gtest_main)
add_test(NAME syntax_error_test COMMAND syntax_error_test)
set_target_properties(syntax_error_test PROPERTIES FOLDER "Tests/Core")

# --- Feature Tests ---
add_executable(include_test IncludeTest.cpp)
target_link_libraries(include_test Yini GTest::gtest_main)
add_test(NAME include_test COMMAND include_test)
set_target_properties(include_test PROPERTIES FOLDER "Tests/Features")

add_executable(dyna_test DynaTest.cpp)
target_link_libraries(dyna_test Yini GTest::gtest_main)
add_test(NAME dyna_test COMMAND dyna_test)
set_target_properties(dyna_test PROPERTIES FOLDER "Tests/Features")

add_executable(serialization_test SerializationTest.cpp)
target_link_libraries(serialization_test Yini GTest::gtest_main)
add_test(NAME serialization_test COMMAND serialization_test)
set_target_properties(serialization_test PROPERTIES FOLDER "Tests/Features")

add_executable(complex_interpreter_test ComplexInterpreterTest.cpp)
target_link_libraries(complex_interpreter_test Yini GTest::gtest_main)
add_test(NAME complex_interpreter_test COMMAND complex_interpreter_test)
set_target_properties(complex_interpreter_test PROPERTIES FOLDER "Tests/Features")

add_executable(xref_test XRefTest.cpp)
target_link_libraries(xref_test Yini GTest::gtest_main)
add_test(NAME xref_test COMMAND xref_test)
set_target_properties(xref_test PROPERTIES FOLDER "Tests/Features")

add_executable(schema_test SchemaTest.cpp)
target_link_libraries(schema_test Yini GTest::gtest_main)
add_test(NAME schema_test COMMAND schema_test)
set_target_properties(schema_test PROPERTIES FOLDER "Tests/Features")

add_executable(data_type_test DataTypeTest.cpp)
target_link_libraries(data_type_test Yini GTest::gtest_main)
add_test(NAME data_type_test COMMAND data_type_test)
set_target_properties(data_type_test PROPERTIES FOLDER "Tests/Features")

# --- CLI Test ---
add_executable(cli_test CliTest.cpp)
target_link_libraries(cli_test Yini GTest::gtest_main)
add_dependencies(cli_test yini-cli)
set_target_properties(cli_test PROPERTIES FOLDER "Tests/CLI")

# Pass the path to the yini-cli executable to the test
target_compile_definitions(cli_test PRIVATE YINI_CLI_PATH="$<TARGET_FILE:yini-cli>")

add_test(NAME cli_test COMMAND cli_test)