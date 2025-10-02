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
    EXPECT_NE(output.find("Error:"), std::string::npos);
    EXPECT_NE(output.find("Could not open file"), std::string::npos);
}

TEST(CliTest, CompileAndDecompile)
{
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

TEST(CliTest, GetValue)
{
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    const std::string yini_file = "cli_get_test.yini";
    std::ofstream(yini_file) << "[Settings]\nvolume = 100";

    std::string cmd = cli_path + " get " + yini_file + " Settings volume";
    std::string output = exec(cmd.c_str());
    EXPECT_EQ(output, "100\n");
}

TEST(CliTest, SetValue)
{
    std::string cli_path = TOSTRING(YINI_CLI_PATH);
    const std::string yini_file = "cli_set_test.yini";
    std::ofstream(yini_file) << "[Settings]\nvolume = Dyna(100)";

    std::string cmd = cli_path + " set " + yini_file + " Settings volume 50";
    std::string output = exec(cmd.c_str());
    EXPECT_NE(output.find("Set 'volume' in section 'Settings'."), std::string::npos);

    std::ifstream infile(yini_file);
    std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("volume = 50"), std::string::npos);
}