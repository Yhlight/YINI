#include "gtest/gtest.h"
#include "CLI/Cooker.h"
#include "Loader/YbinLoader.h"
#include <map>
#include <any>
#include <fstream>
#include <cstdio>

TEST(CookerTests, CookAndLoadSimpleAsset)
{
    // 1. Prepare the data in the flat format that the Resolver produces
    std::map<std::string, std::any> config;
    config["TestSection.intValue"] = 123.0; // Resolver produces doubles
    config["TestSection.boolValue"] = true;
    config["TestSection.stringValue"] = std::string("hello cooked world");

    const std::string outputPath = "test_asset.ybin";

    // 2. Cook the asset
    YINI::Cooker cooker;
    cooker.cook(config, outputPath);

    // 3. Load the asset
    YINI::YbinLoader loader(outputPath);

    // 4. Assert the values
    auto intVal = loader.get_int("TestSection", "intValue");
    ASSERT_TRUE(intVal.has_value());
    EXPECT_EQ(*intVal, 123);

    auto doubleVal = loader.get_double("TestSection", "intValue");
    ASSERT_TRUE(doubleVal.has_value());
    EXPECT_EQ(*doubleVal, 123.0);

    auto boolVal = loader.get_bool("TestSection", "boolValue");
    ASSERT_TRUE(boolVal.has_value());
    EXPECT_EQ(*boolVal, true);

    auto stringVal = loader.get_string("TestSection", "stringValue");
    ASSERT_TRUE(stringVal.has_value());
    EXPECT_EQ(*stringVal, "hello cooked world");

    // Assert non-existent values
    auto nonExistent = loader.get_int("TestSection", "nonExistent");
    EXPECT_FALSE(nonExistent.has_value());

    auto nonExistentSection = loader.get_int("NonExistentSection", "key");
    EXPECT_FALSE(nonExistentSection.has_value());

    // Clean up the created file
    std::remove(outputPath.c_str());
}
