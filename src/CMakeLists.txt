add_library(YINI STATIC
    Lexer/Lexer.cpp
    Parser/Parser.cpp
)

target_include_directories(YINI PUBLIC ../include)