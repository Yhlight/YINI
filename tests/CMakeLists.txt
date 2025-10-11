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

add_executable(resolver_tests ResolverTests.cpp)
target_link_libraries(resolver_tests PRIVATE Resolver Parser Lexer gtest_main)
target_include_directories(resolver_tests PRIVATE ../src)
add_test(NAME resolver_tests COMMAND $<TARGET_FILE:resolver_tests>)

add_executable(parser_bug_tests ParserBugTests.cpp)
target_link_libraries(parser_bug_tests PRIVATE gtest_main Parser Lexer)
target_include_directories(parser_bug_tests PRIVATE ../src)
add_test(NAME parser_bug_tests COMMAND $<TARGET_FILE:parser_bug_tests>)

add_executable(validator_tests ValidatorTests.cpp)
target_link_libraries(validator_tests PRIVATE Validator Resolver Parser Lexer gtest_main)
target_include_directories(validator_tests PRIVATE ../src)
add_test(NAME validator_tests COMMAND $<TARGET_FILE:validator_tests>)

add_executable(ymeta_manager_tests YmetaManagerTests.cpp)
target_link_libraries(ymeta_manager_tests PRIVATE YmetaManager gtest_main)
target_include_directories(ymeta_manager_tests PRIVATE ../src)
add_test(NAME ymeta_manager_tests COMMAND $<TARGET_FILE:ymeta_manager_tests>)

add_executable(interop_tests InteropTests.cpp)
target_link_libraries(interop_tests PRIVATE gtest_main YiniInterop)
target_include_directories(interop_tests PRIVATE ../src)
add_test(NAME interop_tests COMMAND $<TARGET_FILE:interop_tests>)

file(COPY
    "include_test.yini"
    "main_include_test.yini"
    "cooker_test.yini"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(cooker_loader_tests CookerLoaderTests.cpp)
target_link_libraries(cooker_loader_tests PRIVATE gtest_main YiniInterop)
target_include_directories(cooker_loader_tests PRIVATE ../src)
add_test(NAME cooker_loader_tests COMMAND $<TARGET_FILE:cooker_loader_tests>)