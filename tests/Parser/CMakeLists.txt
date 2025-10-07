# Parser tests
add_executable(test_parser test_parser.cpp)

target_link_libraries(test_parser PRIVATE yini_parser yini_lexer)

target_include_directories(test_parser PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME ParserTest COMMAND test_parser)

# Edge cases and error handling tests
add_executable(test_edge_cases test_edge_cases.cpp)

target_link_libraries(test_edge_cases PRIVATE yini_parser yini_lexer)

target_include_directories(test_edge_cases PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME EdgeCasesTest COMMAND test_edge_cases)

# Schema validation tests
add_executable(test_schema test_schema.cpp)

target_link_libraries(test_schema PRIVATE yini_parser yini_lexer)

target_include_directories(test_schema PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME SchemaTest COMMAND test_schema)
