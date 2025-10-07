# Include FetchContent for downloading dependencies
include(FetchContent)

# --- GoogleTest ---
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows:
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG        release-1.12.1
# )
FetchContent_MakeAvailable(googletest)

# --- Test Executable ---
# Create an executable for our tests
add_executable(yini_tests
    parser_test.cpp
)

# Link the test executable against GTest and the main yini library
target_link_libraries(yini_tests PRIVATE
    gtest_main
    yini
)

# --- CTest Integration ---
# Discover and add tests to CTest
include(GoogleTest)
gtest_discover_tests(yini_tests)