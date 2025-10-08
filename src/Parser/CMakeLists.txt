# Placeholder for Parser library
add_library(Parser parser.cpp)

target_link_libraries(Parser PUBLIC Lexer)