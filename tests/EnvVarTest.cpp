#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <cstdlib> // For setenv and unsetenv

// Helper to create a file and load it with a YiniManager
void load_from_source(YINI::YiniManager& manager, const std::string& source) {
    std::ofstream outfile("env_var_test.yini");
    outfile << source;
    outfile.close();
    manager.load("env_var_test.yini");
}

TEST(EnvVarTest, SubstitutesExistingEnvVar) {
    // Set an environment variable for the test
    setenv("YINI_TEST_HOST", "testhost.com", 1);

    YINI::YiniManager manager;
    std::string source = "[Database]\nhost = ${YINI_TEST_HOST}";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("Database", "host");
    ASSERT_TRUE(std::holds_alternative<std::string>(value.m_value));
    EXPECT_EQ(std::get<std::string>(value.m_value), "testhost.com");

    // Clean up the environment variable
    unsetenv("YINI_TEST_HOST");
}

TEST(EnvVarTest, UsesDefaultValueWhenVarIsUnset) {
    // Ensure the variable is not set
    unsetenv("YINI_TEST_PORT");

    YINI::YiniManager manager;
    std::string source = "[Database]\nport = ${YINI_TEST_PORT:8080}";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("Database", "port");
    ASSERT_TRUE(std::holds_alternative<double>(value.m_value));
    EXPECT_EQ(std::get<double>(value.m_value), 8080);
}

TEST(EnvVarTest, ThrowsWhenRequiredVarIsUnset) {
    // Ensure the variable is not set
    unsetenv("YINI_REQUIRED_VAR");

    YINI::YiniManager manager;
    std::string source = "[Config]\nkey = ${YINI_REQUIRED_VAR}";

    try {
        load_from_source(manager, source);
        FAIL() << "Expected a RuntimeError for missing environment variable.";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_STREQ(e.what(), "Required environment variable 'YINI_REQUIRED_VAR' is not set and no default value is provided.");
    } catch (...) {
        FAIL() << "Expected a RuntimeError, but got a different exception.";
    }
}

TEST(EnvVarTest, HandlesComplexDefaultValue) {
    // Ensure the variable is not set
    unsetenv("YINI_COMPLEX_DEFAULT");

    YINI::YiniManager manager;
    std::string source = "[Config]\nvalue = ${YINI_COMPLEX_DEFAULT:10 * (2 + 3)}";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("Config", "value");
    ASSERT_TRUE(std::holds_alternative<double>(value.m_value));
    EXPECT_EQ(std::get<double>(value.m_value), 50);
}