# Lexer library
add_library(yini_lexer
    Lexer.cpp
    Token.cpp
)

target_include_directories(yini_lexer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(yini_lexer PUBLIC cxx_std_17)
