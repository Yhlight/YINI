add_executable(YINITests
    TestLexer.cpp
)

target_link_libraries(YINITests YINI gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(YINITests)