add_library(yini_parser
    Parser.cpp
    Parser.h
    Ast.cpp
    Ast.h
)

target_include_directories(yini_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(yini_parser PUBLIC yini_lexer)