#include "gtest/gtest.h"
#include <cstdlib>
#include <string>
#include <iostream>

#if defined(_WIN32)
#define YINI_EXECUTABLE "..\\bin\\yini.exe"
#else
#define YINI_EXECUTABLE "../bin/yini"
#endif

TEST(CLITests, ValidateCommandSucceedsWithValidConfig)
{
    // Note: The test files are copied to the build directory by CMake.
    std::string command = std::string(YINI_EXECUTABLE) + " validate test_schema.yini test_valid_config.yini";
    int result = std::system(command.c_str());
    // std::system returns a platform-specific value. On POSIX, the exit code is in the high byte.
    // On Windows, it's the direct return value. WEXITSTATUS handles POSIX, and for Windows,
    // if the command executes, it will be 0 for success.
    #if defined(__linux__) || defined(__APPLE__)
    ASSERT_EQ(WEXITSTATUS(result), 0);
    #else
    ASSERT_EQ(result, 0);
    #endif
}

TEST(CLITests, ValidateCommandFailsWithInvalidConfig)
{
    std::string command = std::string(YINI_EXECUTABLE) + " validate test_schema.yini test_invalid_config.yini";
    int result = std::system(command.c_str());
    #if defined(__linux__) || defined(__APPLE__)
    ASSERT_NE(WEXITSTATUS(result), 0);
    #else
    ASSERT_NE(result, 0);
    #endif
}
