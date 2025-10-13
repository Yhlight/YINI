# Lexer tests
add_executable(test_lexer test_lexer.cpp)

target_link_libraries(test_lexer PRIVATE yini_lexer)

target_include_directories(test_lexer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME LexerTest COMMAND test_lexer)
