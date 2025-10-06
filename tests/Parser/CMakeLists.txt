# Parser tests using GoogleTest
add_executable(ParserTests test_parser.cpp)

# Link against the parser, lexer, and gtest_main
target_link_libraries(ParserTests
    PRIVATE
    yini_parser # The static library for the parser
    yini_lexer  # The static library for the lexer
    gtest_main  # GoogleTest with main()
)

# Include the project's public headers
target_include_directories(ParserTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Discover tests for CTest integration
include(GoogleTest)
gtest_discover_tests(ParserTests)