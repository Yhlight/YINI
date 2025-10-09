#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>
#include <nlohmann/json.hpp>

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

class CLITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary YINI file for testing
        std::ofstream test_file("temp_test_cli.yini");
        test_file << R"(
[TestSection]
key1 = "value1"
key2 = 123
key3 = [1, 2, 3]

[AnotherSection]
test_key = true
)";
        test_file.close();
    }

    void TearDown() override {
        // Remove the temporary file
        std::remove("temp_test_cli.yini");
    }
};

TEST_F(CLITest, ExportJson) {
    std::string command = "./build/src/cli/yini --export-json temp_test_cli.yini";
    std::string output = exec(command.c_str());

    auto expected_json = nlohmann::json::parse(R"({
        "AnotherSection": {
            "test_key": true
        },
        "TestSection": {
            "key1": "value1",
            "key2": 123,
            "key3": [1, 2, 3]
        }
    })");

    auto output_json = nlohmann::json::parse(output);

    EXPECT_EQ(output_json, expected_json);
}

TEST_F(CLITest, QueryKey) {
    std::string command = "./build/src/cli/yini --query TestSection.key2 temp_test_cli.yini";
    std::string output = exec(command.c_str());

    auto output_json = nlohmann::json::parse(output);

    EXPECT_EQ(output_json, 123);
}

TEST_F(CLITest, QuerySection) {
    std::string command = "./build/src/cli/yini --query AnotherSection temp_test_cli.yini";
    std::string output = exec(command.c_str());

    auto expected_json = nlohmann::json::parse(R"({
        "test_key": true
    })");

    auto output_json = nlohmann::json::parse(output);

    EXPECT_EQ(output_json, expected_json);
}

TEST_F(CLITest, QueryNonExistentKey) {
    std::string command = "./build/src/cli/yini --query TestSection.nonexistent temp_test_cli.yini";
    std::string output = exec(command.c_str());
    // This should produce an error message on stderr, but for simplicity, we check that stdout is empty.
    // A more robust test would capture stderr.
    EXPECT_TRUE(output.empty());
}

TEST_F(CLITest, QueryNonExistentSection) {
    std::string command = "./build/src/cli/yini --query NonExistentSection temp_test_cli.yini";
    std::string output = exec(command.c_str());
    EXPECT_TRUE(output.empty());
}

TEST_F(CLITest, GenerateYmeta) {
    std::string command = "./build/src/cli/yini --generate-ymeta temp_test_cli.yini";
    exec(command.c_str());

    // Check if the .ymeta file exists
    std::ifstream ymeta_file("temp_test_cli.ymeta");
    ASSERT_TRUE(ymeta_file.good());

    // Verify the content of the .ymeta file
    nlohmann::json ymeta_json;
    ymeta_file >> ymeta_json;

    auto expected_json = nlohmann::json::parse(R"({
        "AnotherSection": {
            "test_key": true
        },
        "TestSection": {
            "key1": "value1",
            "key2": 123,
            "key3": [1, 2, 3]
        }
    })");

    EXPECT_EQ(ymeta_json, expected_json);

    // Clean up the generated .ymeta file
    std::remove("temp_test_cli.ymeta");
}