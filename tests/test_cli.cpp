#include <gtest/gtest.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>

// Helper function to execute a command and get its standard output.
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

// Helper to read file content
static std::string readFile(const std::string& path) {
    std::ifstream t(path);
    if (!t.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

// The path to the CLI executable depends on the build configuration,
// but for this test environment, it's consistently in 'bin/'.
#if defined(_WIN32)
const std::string CLI_PATH = "bin\\yini-cli.exe";
#else
const std::string CLI_PATH = "./bin/yini-cli";
#endif

class CLITest : public ::testing::Test {
protected:
    const std::string get_test_file = "cli_get_test.yini";
    const std::string format_test_file = "cli_format_test.yini";
    const std::string inplace_test_file = "cli_inplace_test.yini";

    void SetUp() override {
        // Clean up any files from previous runs to ensure a clean slate.
        std::remove(get_test_file.c_str());
        std::remove((get_test_file + ".ymeta").c_str());
        std::remove(format_test_file.c_str());
        std::remove((format_test_file + ".ymeta").c_str());
        std::remove(inplace_test_file.c_str());
        std::remove((inplace_test_file + ".ymeta").c_str());
    }

    void TearDown() override {
        // Repeat cleanup after test execution.
        SetUp();
    }
};

TEST_F(CLITest, GetCommand) {
    // 1. Create a test file.
    std::ofstream(get_test_file) << "[Network]\n  host = \"localhost\"\nport=8080";

    // 2. Run the 'get' command for the string value.
    std::string command_str = CLI_PATH + " get Network.host " + get_test_file;
    std::string output_str = exec(command_str.c_str());
    // The valueToString function adds quotes to strings.
    ASSERT_EQ(output_str, "\"localhost\"\n");

    // 3. Run the 'get' command for the integer value.
    std::string command_int = CLI_PATH + " get Network.port " + get_test_file;
    std::string output_int = exec(command_int.c_str());
    ASSERT_EQ(output_int, "8080\n");
}

TEST_F(CLITest, FormatCommand) {
    // 1. Create a poorly formatted test file.
    std::ofstream(format_test_file) << "[Settings]\nkey=value\n  val2 = 123\n\n[Other]\n  foo=bar";

    // 2. Run the 'format' command and capture the output.
    std::string command = CLI_PATH + " format " + format_test_file;
    std::string formatted_output = exec(command.c_str());

    // 3. Define the expected, correctly formatted output.
    const std::string expected_output =
        "[Settings]\n"
        "  key = \"value\"\n"
        "  val2 = 123\n"
        "\n"
        "[Other]\n"
        "  foo = \"bar\"\n";

    ASSERT_EQ(formatted_output, expected_output);
}

TEST_F(CLITest, FormatCommandInPlace) {
    // 1. Create a poorly formatted test file.
    std::ofstream(inplace_test_file) << "[Settings]\nkey=value\n  val2 = 123";

    // 2. Run the 'format --in-place' command.
    std::string command = CLI_PATH + " format " + inplace_test_file + " --in-place";
    exec(command.c_str());

    // 3. Read the file content back.
    std::string file_content = readFile(inplace_test_file);

    // 4. Define the expected, correctly formatted output.
    const std::string expected_output =
        "[Settings]\n"
        "  key = \"value\"\n"
        "  val2 = 123\n";

    ASSERT_EQ(file_content, expected_output);
}