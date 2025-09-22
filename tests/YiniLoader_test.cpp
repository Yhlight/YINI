#include <gtest/gtest.h>
#include "YiniLoader.h"
#include <fstream>
#include <filesystem>

class YiniLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::filesystem::create_directory("test_data");
    }

    void TearDown() override
    {
        std::filesystem::remove_all("test_data");
    }
};

TEST_F(YiniLoaderTest, IncludeFile)
{
    std::ofstream base_file("test_data/base.yini");
    base_file << "[BaseSection]\n";
    base_file << "base_key = 123\n";
    base_file.close();

    std::ofstream main_file("test_data/main.yini");
    main_file << "[#include]\n";
    main_file << "+= \"base.yini\"\n";
    main_file << "[MainSection]\n";
    main_file << "main_key = 456\n";
    main_file.close();

    Yini::YiniLoader loader;
    Yini::YiniData data = loader.loadFile("test_data/main.yini");

    auto base_section = data.getSection("BaseSection");
    ASSERT_NE(base_section, nullptr);
    EXPECT_EQ(base_section->getKeyValues().at("base_key").get<Yini::YiniInteger>(), 123);

    auto main_section = data.getSection("MainSection");
    ASSERT_NE(main_section, nullptr);
    EXPECT_EQ(main_section->getKeyValues().at("main_key").get<Yini::YiniInteger>(), 456);
}

TEST_F(YiniLoaderTest, Inheritance)
{
    std::ofstream main_file("test_data/main.yini");
    main_file << "[Base]\n";
    main_file << "key1 = 1\n";
    main_file << "key2 = 2\n";
    main_file << "[Derived] : Base\n";
    main_file << "key2 = 3\n";
    main_file << "key3 = 4\n";
    main_file.close();

    Yini::YiniLoader loader;
    Yini::YiniData data = loader.loadFile("test_data/main.yini");

    auto derived_section = data.getSection("Derived");
    ASSERT_NE(derived_section, nullptr);

    auto kvs = derived_section->getKeyValues();
    EXPECT_EQ(kvs.size(), 3);
    EXPECT_EQ(kvs.at("key1").get<Yini::YiniInteger>(), 1);
    EXPECT_EQ(kvs.at("key2").get<Yini::YiniInteger>(), 3);
    EXPECT_EQ(kvs.at("key3").get<Yini::YiniInteger>(), 4);
}
