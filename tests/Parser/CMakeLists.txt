# Parser tests
add_executable(test_parser test_parser.cpp)

target_link_libraries(test_parser PRIVATE yini_parser yini_lexer)

target_include_directories(test_parser PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME ParserTest COMMAND test_parser)
