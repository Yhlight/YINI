# Parser library
add_library(yini_parser
    Parser.cpp
    Value.cpp
)

target_link_libraries(yini_parser PUBLIC yini_lexer)

target_include_directories(yini_parser PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(yini_parser PUBLIC cxx_std_17)
