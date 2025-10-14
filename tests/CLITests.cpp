#include "gtest/gtest.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// Helper function to execute a command and get its output
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

TEST(CLITests, CookAndDecompileRoundTrip) {
    const char* yini_filename = "cli_roundtrip_test.yini";
    const char* ybin_filename = "cli_roundtrip_test.ybin";

    // 1. Create a test .yini file
    std::ofstream yini_file(yini_filename);
    yini_file << "[TestSection]\n";
    yini_file << "key1 = \"value1\"\n";
    yini_file << "key2 = 123\n";
    yini_file.close();

    // Find the yini executable
    // Note: CTest runs tests from the build/tests directory, so we need to go up one level.
    const char* yini_executable = "../bin/yini";

    // 2. Run the 'cook' command
    std::string cook_command = std::string(yini_executable) + " cook -o " + ybin_filename + " " + yini_filename;
    std::string cook_output = exec(cook_command.c_str());
    ASSERT_TRUE(cook_output.find("Successfully cooked") != std::string::npos);

    // 3. Run the 'decompile' command and capture its output
    std::string decompile_command = std::string(yini_executable) + " decompile " + ybin_filename;
    std::string decompile_output = exec(decompile_command.c_str());

    // 4. Verify the decompiled output
    // The output format might have different whitespace, so we just check for key parts.
    ASSERT_TRUE(decompile_output.find("[TestSection]") != std::string::npos);
    ASSERT_TRUE(decompile_output.find("key1 = \"value1\"") != std::string::npos);
    ASSERT_TRUE(decompile_output.find("key2 = 123") != std::string::npos);

    // 5. Cleanup
    remove(yini_filename);
    remove(ybin_filename);
}
