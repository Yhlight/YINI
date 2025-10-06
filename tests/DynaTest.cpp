#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/DynaValue.h"
#include <fstream>
#include <sstream>
#include <string>
#include <variant>

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

    const auto& section = manager.get_interpreter().resolved_sections.at("MySection");
    ASSERT_EQ(section.count("my_dynamic_val"), 1);

    const auto& value = section.at("my_dynamic_val");
    auto* dyna_val_ptr = std::get_if<std::unique_ptr<YINI::DynaValue>>(&value.m_value);
    ASSERT_NE(dyna_val_ptr, nullptr);

    const auto& inner_value = (*dyna_val_ptr)->get();
    EXPECT_EQ(std::get<double>(inner_value.m_value), 123);
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
    EXPECT_EQ(std::get<double>(manager.get_value("Settings", "volume").m_value), 100);

    // Set a new value
    manager.set_value("Settings", "volume", 75.0);
    EXPECT_EQ(std::get<double>(manager.get_value("Settings", "volume").m_value), 75.0);

    // 2. Save the changes
    manager.save_changes();

    // 3. Re-load the file with a new manager and verify the content semantically
    YINI::YiniManager verify_manager;
    verify_manager.load(filepath);

    // Check that the dynamic value was updated
    EXPECT_EQ(std::get<double>(verify_manager.get_value("Settings", "volume").m_value), 75.0);

    // Check that the non-dynamic value is still correct
    EXPECT_EQ(std::get<double>(verify_manager.get_value("Settings", "brightness").m_value), 80.0);

    // Check that comments were preserved correctly
    EXPECT_EQ(verify_manager.get_section_doc_comment("Settings"), " This is a test file for dynamic values.");
    EXPECT_EQ(verify_manager.get_key_inline_comment("Settings", "volume"), " Master volume");
    EXPECT_EQ(verify_manager.get_key_inline_comment("Settings", "brightness"), " A non-dynamic value");
}