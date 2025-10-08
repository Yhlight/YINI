cmake_minimum_required(VERSION 3.10)

project(YiniTests)

# Add the test executable
add_executable(yini_tests LexerTests.cpp ParserTests.cpp AdvancedParserTests.cpp)

# Link the core library to the tests
target_link_libraries(yini_tests PRIVATE yini_core)

# Include the vendored doctest header
target_include_directories(yini_tests PRIVATE ${CMAKE_SOURCE_DIR}/extern)

# Enable testing and add the test
enable_testing()
add_test(NAME yini_tests COMMAND yini_tests)