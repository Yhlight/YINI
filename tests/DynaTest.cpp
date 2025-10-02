#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/DynaValue.h"
#include <fstream>
#include <sstream>
#include <string>

// Helper to read a file's content into a string
std::string get_file_contents(const std::string& path) {
    std::ifstream infile(path);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    return buffer.str();
}

TEST(DynaTest, InterpreterCreatesDynaValue) {
    std::string source = R"(
        [MySection]
        my_dynamic_val = Dyna(123)
    )";
    std::ofstream("dyna_interpreter_test.yini") << source;

    YINI::YiniManager manager;
    manager.load("dyna_interpreter_test.yini");

    const auto& section = manager.interpreter.resolved_sections["MySection"];
    ASSERT_EQ(section.count("my_dynamic_val"), 1);

    const auto& value = section.at("my_dynamic_val");
    ASSERT_EQ(value.type(), typeid(YINI::DynaValue));

    auto dyna_val = std::any_cast<YINI::DynaValue>(value);
    EXPECT_EQ(std::any_cast<double>(dyna_val.get()), 123);
}

TEST(DynaTest, NonDestructiveWriteBack) {
    const std::string filepath = "dyna_writeback_test.yini";
    std::string original_content = R"(
        // This is a test file for dynamic values.
        [Settings]
        volume = Dyna(100) // Master volume
        brightness = 80 // A non-dynamic value
    )";
    std::ofstream(filepath) << original_content;

    // 1. Load the file and change a dynamic value
    YINI::YiniManager manager;
    manager.load(filepath);

    // Verify initial value
    EXPECT_EQ(std::any_cast<double>(manager.get_value("Settings", "volume")), 100);

    // Set a new value
    manager.set_value("Settings", "volume", 75.0);
    EXPECT_EQ(std::any_cast<double>(manager.get_value("Settings", "volume")), 75.0);

    // 2. Save the changes
    manager.save_changes();

    // 3. Read the file back and verify its contents
    std::string new_content = get_file_contents(filepath);

    // Check that the dynamic value was updated
    EXPECT_NE(new_content.find("volume = 75 "), std::string::npos);

    // Check that comments and other lines are preserved
    EXPECT_NE(new_content.find("// This is a test file for dynamic values."), std::string::npos);
    EXPECT_NE(new_content.find("// Master volume"), std::string::npos);
    EXPECT_NE(new_content.find("brightness = 80"), std::string::npos);
}