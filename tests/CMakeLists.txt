include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(
  YiniTests
  Lexer_test.cpp
  Parser_test.cpp
  YiniLoader_test.cpp
  YmetaSerializer_test.cpp
)

target_link_libraries(
  YiniTests
  PRIVATE
  YiniLib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(YiniTests)

add_test(
    NAME CLITests
    COMMAND ${CMAKE_SOURCE_DIR}/tests/cli_test.sh $<TARGET_FILE:yini>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
