# Lexer tests using GoogleTest
add_executable(LexerTests test_lexer.cpp)

# Link against the lexer library and gtest_main
# gtest_main provides a main() function, so our test code doesn't need one.
target_link_libraries(LexerTests
    PRIVATE
    yini_lexer # The static library for the lexer
    gtest_main # GoogleTest with main()
)

# Include the project's public headers
target_include_directories(LexerTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Discover tests for CTest integration
include(GoogleTest)
gtest_discover_tests(LexerTests)