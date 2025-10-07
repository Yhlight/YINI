# Find the GoogleTest library
find_package(GTest REQUIRED)

# Create the test executable
add_executable(yini_tests
    test_main.cpp
    test_parser.cpp
)

# Link the test executable against GoogleTest and the YINI library
target_link_libraries(yini_tests
    GTest::GTest
    GTest::Main
    yini
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(yini_tests)