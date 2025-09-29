include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1
)

FetchContent_MakeAvailable(googletest)

add_executable(yini_tests
    test_parser.cpp
    test_serializer.cpp
    test_yini_serializer.cpp
    test_manager.cpp
    test_deserializer.cpp
)

target_link_libraries(yini_tests yini gtest_main)

include(GoogleTest)
gtest_discover_tests(yini_tests)