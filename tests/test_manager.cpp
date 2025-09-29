#include "YINI/YiniData.hpp"
#include "YINI/YiniManager.hpp"
#include <cstdio> // For std::remove
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

// Helper to read file content
static std::string readFileContent(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
    return "";
  return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

// Helper to create a file and set its last write time
void createFileWithTimestamp(const std::string& path, const std::string& content, fs::file_time_type timestamp) {
    std::ofstream(path) << content;
    fs::last_write_time(path, timestamp);
}


TEST(YiniManagerTest, LoadFromFileCreatesYmeta)
{
  const std::string yiniPath = "manager_test.yini";
  const std::string ymetaPath = "manager_test.ymeta";

  // Create a dummy file in the build/bin directory where the test runs
  std::ofstream(yiniPath) << "[Test]\nvalue = \"Hello\"";

  // Clean up any previous ymeta file
  fs::remove(ymetaPath);

  YINI::YiniManager manager(yiniPath);
  const auto &doc = manager.getDocument();

  // Check that the document was loaded correctly
  const auto *section = doc.findSection("Test");
  ASSERT_NE(section, nullptr);

  auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
                         [](const auto &p) { return p.key == "value"; });
  ASSERT_NE(it, section->pairs.end());
  EXPECT_EQ(std::get<std::string>(it->value.data), "Hello");

  // Check that the .ymeta file was created
  ASSERT_TRUE(fs::exists(ymetaPath));
  std::string ymetaContent = readFileContent(ymetaPath);
  ASSERT_FALSE(ymetaContent.empty());

  // Check if the content is valid JSON (basic check)
  EXPECT_NE(ymetaContent.find("\"sections\""), std::string::npos);
  EXPECT_NE(ymetaContent.find("\"Test\""), std::string::npos);
  EXPECT_NE(ymetaContent.find("\"pairs\""), std::string::npos);
  EXPECT_NE(ymetaContent.find("\"value\":\"Hello\""), std::string::npos);

  // Clean up the created files
  fs::remove(yiniPath);
  fs::remove(ymetaPath);
}

TEST(YiniManagerTest, SetValueCreatesBackups)
{
  const std::string yiniPath = "backup_test.yini";
  const std::string ymetaPath = "backup_test.ymeta";

  // Create a dummy file
  std::ofstream(yiniPath) << "[Data]\nvalue = 0";

  // Clean up any previous files
  fs::remove(ymetaPath);
  for (int i = 1; i <= 6; ++i)
  {
    fs::remove(ymetaPath + ".bak" + std::to_string(i));
  }

  YINI::YiniManager manager(yiniPath);

  // Modify the value 6 times to trigger backup rotation
  for (int i = 1; i <= 6; ++i)
  {
    manager.setIntValue("Data", "value", i);
  }

  // After 6 saves, we expect the main .ymeta and 5 backup files
  EXPECT_TRUE(readFileContent(ymetaPath).find("\"value\":6") != std::string::npos);
  EXPECT_TRUE(readFileContent(ymetaPath + ".bak1").find("\"value\":5") != std::string::npos);
  EXPECT_TRUE(readFileContent(ymetaPath + ".bak2").find("\"value\":4") != std::string::npos);
  EXPECT_TRUE(readFileContent(ymetaPath + ".bak3").find("\"value\":3") != std::string::npos);
  EXPECT_TRUE(readFileContent(ymetaPath + ".bak4").find("\"value\":2") != std::string::npos);
  EXPECT_TRUE(readFileContent(ymetaPath + ".bak5").find("\"value\":1") != std::string::npos);

  // The 6th backup should not exist
  EXPECT_FALSE(fs::exists(ymetaPath + ".bak6"));


  // Clean up all created files
  fs::remove(yiniPath);
  fs::remove(ymetaPath);
  for (int i = 1; i <= 5; ++i)
  {
    fs::remove(ymetaPath + ".bak" + std::to_string(i));
  }
}

TEST(YiniManagerTest, SetValueSavesToYmeta)
{
  const std::string yiniPath = "autosave_test.yini";
  const std::string ymetaPath = "autosave_test.ymeta";

  // Create a dummy file
  std::ofstream(yiniPath) << "[Settings]\nvolume = 100";
  fs::remove(ymetaPath); // Ensure no old cache exists

  // Load the file, which creates the initial .ymeta
  YINI::YiniManager manager(yiniPath);

  // Modify a value, which should trigger an auto-save
  manager.setIntValue("Settings", "volume", 75);

  // Read the ymeta file from disk to check if it was updated
  std::string ymetaContent = readFileContent(ymetaPath);
  ASSERT_FALSE(ymetaContent.empty());
  EXPECT_NE(ymetaContent.find("\"volume\":75"), std::string::npos);

  // Clean up
  fs::remove(yiniPath);
  fs::remove(ymetaPath);
}

TEST(YiniManagerTest, LoadFromFilePrioritizesYmetaCache)
{
  const std::string yiniPath = "cache_test.yini";
  const std::string ymetaPath = "cache_test.ymeta";

  // Create a yini file
  std::ofstream(yiniPath) << "[CachedSection]\nvalue = \"from_yini_file_should_be_ignored\"";

  // Create a newer ymeta file
  const std::string new_cache_content = R"({
        "defines": {}, "sections": { "CachedSection": { "inherits": [], "pairs": { "value": "from_cache" }, "register": [] } }
    })";
  std::ofstream(ymetaPath) << new_cache_content;

  fs::last_write_time(ymetaPath, fs::last_write_time(yiniPath) + std::chrono::seconds(1));

  YINI::YiniManager manager(yiniPath);
  const auto &doc = manager.getDocument();

  const auto *section = doc.findSection("CachedSection");
  ASSERT_NE(section, nullptr);

  auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
                         [](const auto &p) { return p.key == "value"; });
  ASSERT_NE(it, section->pairs.end());
  EXPECT_EQ(std::get<std::string>(it->value.data), "from_cache");

  // Clean up the created files
  fs::remove(yiniPath);
  fs::remove(ymetaPath);
}

TEST(YiniManagerTest, IgnoresStaleCache)
{
    const std::string yiniPath = "stale_cache_test.yini";
    const std::string ymetaPath = "stale_cache_test.ymeta";

    // 1. Create an initial .yini and a stale .ymeta
    std::ofstream(ymetaPath) << "[Test]\nvalue = \"stale_cache\"";
    std::ofstream(yiniPath) << "[Test]\nvalue = \"updated_yini\"";

    // Ensure yini is newer than ymeta
    fs::last_write_time(yiniPath, fs::last_write_time(ymetaPath) + std::chrono::seconds(1));

    // 2. Create a manager instance to trigger the load logic
    YINI::YiniManager manager(yiniPath);
    ASSERT_TRUE(manager.isLoaded());
    const auto& doc = manager.getDocument();

    // 3. Verify that the loaded data is from the updated .yini, not the stale cache
    const auto* section = doc.findSection("Test");
    ASSERT_NE(section, nullptr);
    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
                           [](const auto& p) { return p.key == "value"; });
    ASSERT_NE(it, section->pairs.end());
    EXPECT_EQ(std::get<std::string>(it->value.data), "updated_yini");

    // Clean up
    fs::remove(yiniPath);
    fs::remove(ymetaPath);
}