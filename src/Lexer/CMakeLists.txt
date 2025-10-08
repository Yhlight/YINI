add_library(yini_lexer
    Lexer.cpp
    Lexer.h
    Token.h
)

target_include_directories(yini_lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)