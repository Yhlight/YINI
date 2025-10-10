cmake_minimum_required(VERSION 3.14)

project(YINITests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(lexer_tests LexerTests.cpp)

target_link_libraries(lexer_tests PRIVATE Lexer gtest_main)
target_include_directories(lexer_tests PRIVATE ../src)
add_test(NAME lexer_tests COMMAND $<TARGET_FILE:lexer_tests>)

add_executable(parser_tests ParserTests.cpp)
target_link_libraries(parser_tests PRIVATE Parser Lexer gtest_main)
target_include_directories(parser_tests PRIVATE ../src)
add_test(NAME parser_tests COMMAND $<TARGET_FILE:parser_tests>)