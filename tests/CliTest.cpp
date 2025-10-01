#include <gtest/gtest.h>
#include <string>
#include <array>
#include <memory>
#include <cstdio>
#include <fstream>

// Macros to convert the preprocessor definition to a string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

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

TEST(CliTest, CheckValidFile)
{
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    std::ofstream("valid_cli_test.yini") << "[Section]\nkey = value";
    std::string cmd = cli_path + " check valid_cli_test.yini";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("File 'valid_cli_test.yini' is valid."), std::string::npos);
}

TEST(CliTest, CheckInvalidFile)
{
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    std::ofstream("invalid_cli_test.yini") << "[Section\nkey = value";
    std::string cmd = cli_path + " check invalid_cli_test.yini 2>&1";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("Error:"), std::string::npos);
}

TEST(CliTest, HandlesNonExistentFile)
{
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    std::string cmd = cli_path + " check non_existent_file.yini 2>&1";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("Error: Could not open file"), std::string::npos);
}