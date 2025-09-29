include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1
)

FetchContent_MakeAvailable(googletest)

add_executable(yini_tests
    test_parser.cpp
    test_parser_errors.cpp
    test_manager.cpp
    test_roundtrip_and_inheritance.cpp
)

# Pass the path to the test data directory to the tests as a C-string literal
target_compile_definitions(yini_tests PRIVATE
    "YINI_TEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

target_link_libraries(yini_tests yini gtest_main)

include(GoogleTest)
gtest_discover_tests(yini_tests)