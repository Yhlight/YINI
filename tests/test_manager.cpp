#include <gtest/gtest.h>
#include "YINI/YiniManager.hpp"
#include "YINI/YiniData.hpp"
#include <fstream>
#include <string>
#include <cstdio> // For std::remove

// Helper to read file content
static std::string readFileContent(const std::string& path) {
    std::ifstream t(path);
    if (!t.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

TEST(YiniManagerTest, LoadFromFileCreatesYmeta)
{
    const std::string yiniPath = "manager_test.yini";
    const std::string ymetaPath = "manager_test.ymeta";

    // Create a dummy file in the build/bin directory where the test runs
    std::ofstream(yiniPath) << "[Test]\nvalue = \"Hello\"";

    // Clean up any previous ymeta file
    std::remove(ymetaPath.c_str());

    YINI::YiniManager manager(yiniPath);
    const auto& doc = manager.getDocument();

    // Check that the document was loaded correctly
    const auto* section = doc.findSection("Test");
    ASSERT_NE(section, nullptr);

    auto it = std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto& p){ return p.key == "value"; });
    ASSERT_NE(it, section->pairs.end());
    EXPECT_EQ(std::get<std::string>(it->value.data), "Hello");

    // Check that the .ymeta file was created
    std::string ymetaContent = readFileContent(ymetaPath);
    ASSERT_FALSE(ymetaContent.empty());

    // Check if the content is valid JSON (basic check)
    EXPECT_EQ(ymetaContent.front(), '{');
    EXPECT_EQ(ymetaContent.back(), '}');
    EXPECT_NE(ymetaContent.find("\"Test\""), std::string::npos);
    EXPECT_NE(ymetaContent.find("\"value\":\"Hello\""), std::string::npos);

    // Clean up the created files
    std::remove(yiniPath.c_str());
    std::remove(ymetaPath.c_str());
}

TEST(YiniManagerTest, SetValueSavesToYmeta)
{
    const std::string yiniPath = "autosave_test.yini";
    const std::string ymetaPath = "autosave_test.ymeta";

    // Create a dummy file
    std::ofstream(yiniPath) << "[Settings]\nvolume = 100";
    std::remove(ymetaPath.c_str()); // Ensure no old cache exists

    // Load the file, which creates the initial .ymeta
    YINI::YiniManager manager(yiniPath);

    // Modify a value, which should trigger an auto-save
    manager.setIntValue("Settings", "volume", 75);

    // Read the ymeta file from disk to check if it was updated
    std::string ymetaContent = readFileContent(ymetaPath);
    ASSERT_FALSE(ymetaContent.empty());
    EXPECT_NE(ymetaContent.find("\"volume\":75"), std::string::npos);

    // Clean up
    std::remove(yiniPath.c_str());
    std::remove(ymetaPath.c_str());
}

TEST(YiniManagerTest, LoadFromFilePrioritizesYmetaCache)
{
    const std::string yiniPath = "cache_test.yini";
    const std::string ymetaPath = "cache_test.ymeta";

    // Create dummy files for the test
    std::ofstream(yiniPath) << "[CachedSection]\nvalue = \"from_yini_file_should_be_ignored\"";
    std::ofstream(ymetaPath) << "{\"CachedSection\":{\"value\":\"from_cache\"}}";

    YINI::YiniManager manager(yiniPath);
    const auto& doc = manager.getDocument();

    // Check that the document was loaded from the .ymeta cache, not the .yini file.
    const auto* section = doc.findSection("CachedSection");
    ASSERT_NE(section, nullptr);

    auto it = std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto& p){ return p.key == "value"; });
    ASSERT_NE(it, section->pairs.end());
    EXPECT_EQ(std::get<std::string>(it->value.data), "from_cache");

    // Clean up the created files
    std::remove(yiniPath.c_str());
    std::remove(ymetaPath.c_str());
}