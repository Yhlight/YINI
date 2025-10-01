add_library(Yini
    Lexer/Lexer.cpp
    Parser/Parser.cpp
)

target_include_directories(Yini PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})