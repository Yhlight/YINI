# Define the test executable
add_executable(yini_tests main.cpp LexerTests.cpp)

# Link against GTest and our core library
target_link_libraries(yini_tests
    PRIVATE
        gtest_main
        yini_core
)

# Include the test in CTest
include(GoogleTest)
gtest_discover_tests(yini_tests)