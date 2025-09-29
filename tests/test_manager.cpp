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

    YINI::YiniDocument doc;
    bool success = YINI::YiniManager::loadFromFile(yiniPath, doc);

    ASSERT_TRUE(success);

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

    // Clean up the created ymeta file
    std::remove(ymetaPath.c_str());
}

TEST(YiniManagerTest, LoadFromFilePrioritizesYmetaCache)
{
    const std::string yiniPath = "cache_test.yini";
    const std::string ymetaPath = "cache_test.ymeta";

    // Create dummy files for the test
    std::ofstream(yiniPath) << "[CachedSection]\nvalue = \"from_yini_file_should_be_ignored\"";
    std::ofstream(ymetaPath) << "{\"CachedSection\":{\"value\":\"from_cache\"}}";

    YINI::YiniDocument doc;
    bool success = YINI::YiniManager::loadFromFile(yiniPath, doc);

    ASSERT_TRUE(success);

    // Check that the document was loaded from the .ymeta cache, not the .yini file.
    const auto* section = doc.findSection("CachedSection");
    ASSERT_NE(section, nullptr);

    auto it = std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto& p){ return p.key == "value"; });
    ASSERT_NE(it, section->pairs.end());
    EXPECT_EQ(std::get<std::string>(it->value.data), "from_cache");
}