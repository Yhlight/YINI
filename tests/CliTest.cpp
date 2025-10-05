#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

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

TEST(CliTest, CheckValidFile) {
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    std::ofstream("valid_cli_test.yini") << "[Section]\nkey = value";
    std::string cmd = cli_path + " check valid_cli_test.yini";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("File 'valid_cli_test.yini' is syntactically valid."), std::string::npos);
}

TEST(CliTest, CheckInvalidFile) {
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    const std::string filename = "invalid_cli_test.yini";
    std::ofstream(filename) << "[Section\nkey = value";
    std::string cmd = cli_path + " check " + filename + " 2>&1";
    std::string output = exec(cmd.c_str());

    // The parser expects a ']' after the section name but finds 'key' on the next line.
    std::string expected_error = "[" + filename + ":2:1] Error: Expect ']' after section name.";
    EXPECT_NE(output.find(expected_error), std::string::npos);
}

TEST(CliTest, HandlesNonExistentFile) {
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    std::string cmd = cli_path + " check non_existent_file.yini 2>&1";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("Error: Could not open file"), std::string::npos);
}

TEST(CliTest, CompileAndDecompile) {
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    const std::string yini_file = "cli_compile_test.yini";
    const std::string ymeta_file = "cli_compile_test.ymeta";

    // 1. Create a test .yini file
    std::ofstream(yini_file) << "[Test]\nkey = \"hello\"";

    // 2. Run the compile command
    std::string compile_cmd = cli_path + " compile " + yini_file + " " + ymeta_file;
    std::string compile_output = exec(compile_cmd.c_str());
    EXPECT_NE(compile_output.find("Compiled"), std::string::npos);

    // 3. Run the decompile command
    std::string decompile_cmd = cli_path + " decompile " + ymeta_file;
    std::string decompile_output = exec(decompile_cmd.c_str());

    // 4. Check the decompiled output
    EXPECT_NE(decompile_output.find("[Test]"), std::string::npos);
    EXPECT_NE(decompile_output.find("key: \"hello\""), std::string::npos);
}