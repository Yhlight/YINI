#include <gtest/gtest.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>

// A helper function to execute a command and get its output.
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Helper macro to stringify preprocessor definitions
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Test fixture for CLI tests
class CLITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a dummy YINI file for testing
        std::ofstream outfile("test.yini");
        outfile << "[Section1]" << std::endl;
        outfile << "key1 = value1" << std::endl;
        outfile.close();
    }

    void TearDown() override {
        remove("test.yini");
    }
};

TEST_F(CLITest, ParseCommand) {
    std::string cli_path = TOSTRING(YINI_CLI_EXECUTABLE);
    std::string command = cli_path + " parse test.yini";
    std::string output = exec(command.c_str());
    EXPECT_NE(output.find("Successfully parsed file: test.yini"), std::string::npos);
    EXPECT_NE(output.find("--- AST ---"), std::string::npos);
    EXPECT_NE(output.find("Section: Section1"), std::string::npos);
    EXPECT_NE(output.find("  key1 = ..."), std::string::npos);
}

TEST_F(CLITest, CheckCommand) {
    std::string cli_path = TOSTRING(YINI_CLI_EXECUTABLE);
    std::string command = cli_path + " check test.yini";
    std::string output = exec(command.c_str());
    EXPECT_NE(output.find("Syntax check passed for file: test.yini"), std::string::npos);
}

TEST_F(CLITest, CompileAndDecompileCommand) {
    std::string cli_path = TOSTRING(YINI_CLI_EXECUTABLE);
    std::string compile_command = cli_path + " compile test.yini test.ymeta";
    std::string compile_output = exec(compile_command.c_str());
    EXPECT_NE(compile_output.find("Successfully compiled test.yini to test.ymeta"), std::string::npos);

    // Now test decompile
    std::string decompile_command = cli_path + " decompile test.ymeta test_decompiled.yini";
    std::string decompile_output = exec(decompile_command.c_str());
    EXPECT_NE(decompile_output.find("Successfully decompiled test.ymeta to test_decompiled.yini"), std::string::npos);
}