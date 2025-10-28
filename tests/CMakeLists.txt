cmake_minimum_required(VERSION 3.15)

# Test executable
add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer yini_lexer yini_core)

add_executable(test_core test_core.cpp)
target_link_libraries(test_core yini_core)

# Add tests to CTest
add_test(NAME LexerTests COMMAND test_lexer)
add_test(NAME CoreTests COMMAND test_core)
