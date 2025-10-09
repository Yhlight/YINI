#include <gtest/gtest.h>
#include "Parser/parser.h"
#include "cli/repl.h" // Include the new REPL header
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

TEST(ReplTest, GetCommand) {
    Config config;
    config["Test"]["key"] = "value";
    std::string result = process_repl_command("get Test.key", config, "");

    nlohmann::json j = nlohmann::json::parse(result);
    EXPECT_EQ(j, "value");
}

TEST(ReplTest, SetCommand) {
    Config config;
    std::string result = process_repl_command("set Test.key \"new_value\"", config, "");
    EXPECT_EQ(result, "Value set.");
    ASSERT_TRUE(config.count("Test"));
    ASSERT_TRUE(config["Test"].count("key"));
    EXPECT_EQ(std::get<std::string>(config["Test"]["key"]), "new_value");
}

TEST(ReplTest, SaveCommand) {
    Config config;
    config["Test"]["key"] = "value";
    std::string filepath = "test_repl_save.yini";
    std::string result = process_repl_command("save", config, filepath);
    EXPECT_EQ(result, "Configuration saved to " + filepath);

    // Verify file content
    std::ifstream file(filepath);
    ASSERT_TRUE(file.good());
    std::stringstream buffer;
    buffer << file.rdbuf();
    EXPECT_NE(buffer.str().find("[Test]"), std::string::npos);
    EXPECT_NE(buffer.str().find("key = \"value\""), std::string::npos);

    std::remove(filepath.c_str());
}

TEST(ReplTest, HelpCommand) {
    Config config;
    std::string result = process_repl_command("help", config, "");
    EXPECT_NE(result.find("Available commands:"), std::string::npos);
}

TEST(ReplTest, UnknownCommand) {
    Config config;
    std::string result = process_repl_command("foo", config, "");
    EXPECT_EQ(result, "Unknown command: foo");
}