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
    test_c_api.cpp
    test_concurrency.cpp
)

target_link_libraries(yini_tests yini gtest_main)

# --- Improvement: Copy test assets to the build directory ---
# This makes tests more robust by ensuring they can find their data files.

# 1. Glob all .yini files in the current test directory.
file(GLOB YINI_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/*.yini")

# 2. Define the destination directory for the assets.
set(ASSETS_DESTINATION "${CMAKE_BINARY_DIR}/bin/test_assets")

# 3. Add a custom command to copy the assets.
# This command runs *after* the yini_tests target is built.
add_custom_command(
    TARGET yini_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ASSETS_DESTINATION}
    COMMAND ${CMAKE_COMMAND} -E copy ${YINI_ASSETS} ${ASSETS_DESTINATION}
    COMMENT "Copying test assets to ${ASSETS_DESTINATION}"
)
# --- End Improvement ---

include(GoogleTest)
gtest_discover_tests(yini_tests)