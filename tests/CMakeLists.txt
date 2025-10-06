add_executable(YINITests
    TestLexer.cpp
    TestLexerExtended.cpp
    TestLexerLiterals.cpp
    TestLexerSections.cpp
    TestParser.cpp
    TestParserExpressions.cpp
    TestParserMacros.cpp
)

target_link_libraries(YINITests YINI gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(YINITests)