#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include <cstdlib>
#include <fstream>

TEST(EnvironmentTest, SubstitutesEnvironmentVariable) {
    // Set an environment variable for the test
    const char* var_name = "YINI_TEST_VAR";
    const char* var_value = "hello_from_env";
    setenv(var_name, var_value, 1);

    std::string yini_content = "[Test]\nvalue = ${YINI_TEST_VAR}";

    YINI::YiniManager manager;
    const std::string test_file = "env_test.yini";
    std::ofstream(test_file) << yini_content;
    manager.load(test_file);

    std::string result = std::any_cast<std::string>(manager.get_value("Test", "value"));
    EXPECT_EQ(result, var_value);

    // Unset the variable to clean up
    unsetenv(var_name);
}

TEST(EnvironmentTest, HandlesUnsetVariable) {
    // Ensure the variable is not set
    const char* var_name = "YINI_UNSET_VAR";
    unsetenv(var_name);

    std::string yini_content = "[Test]\nvalue = ${YINI_UNSET_VAR}";

    YINI::YiniManager manager;
    const std::string test_file = "env_test_unset.yini";
    std::ofstream(test_file) << yini_content;
    manager.load(test_file);

    std::string result = std::any_cast<std::string>(manager.get_value("Test", "value"));
    EXPECT_EQ(result, ""); // Expect an empty string for unset variables
}