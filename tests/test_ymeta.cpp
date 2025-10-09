#include <gtest/gtest.h>
#include "Ymeta/YmetaManager.h"
#include "Parser/parser.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

TEST(YmetaTest, DynaValueSerialization) {
    // 1. Setup: Create a config with a DynaValue
    std::string input = "[Dynamic]\nlevel = Dyna(10)";
    Parser parser;
    Config config = parser.parse(input);

    // Manually add backup values to the DynaValue for testing purposes
    ASSERT_TRUE(config.count("Dynamic") && config["Dynamic"].count("level"));
    auto& dyna_variant = config["Dynamic"]["level"];
    auto* dyna_ptr = std::get<std::unique_ptr<DynaValue>>(dyna_variant).get();

    dyna_ptr->backup.push_back(std::make_unique<ConfigValue>(9));
    dyna_ptr->backup.push_back(std::make_unique<ConfigValue>(8));

    // 2. Action: Write the .ymeta file
    std::string yini_filepath = "test_dyna.yini";
    YmetaManager ymeta_manager;
    ymeta_manager.write(yini_filepath, config);

    // 3. Verification: Read and check the .ymeta file
    std::string ymeta_filepath = "test_dyna.ymeta";
    std::ifstream ymeta_file(ymeta_filepath);
    ASSERT_TRUE(ymeta_file.good());

    nlohmann::json ymeta_json;
    ymeta_file >> ymeta_json;

    // Check the structure of the DynaValue JSON
    auto& dyna_json = ymeta_json["Dynamic"]["level"];
    EXPECT_EQ(dyna_json["__type"], "DynaValue");
    EXPECT_EQ(dyna_json["value"], 10);

    ASSERT_TRUE(dyna_json["backup"].is_array());
    ASSERT_EQ(dyna_json["backup"].size(), 2);
    EXPECT_EQ(dyna_json["backup"][0], 9);
    EXPECT_EQ(dyna_json["backup"][1], 8);

    // 4. Teardown: Clean up the generated file
    ymeta_file.close();
    std::remove(ymeta_filepath.c_str());
}

TEST(YmetaTest, YmetaLoadingAndCacheInvalidation) {
    std::string yini_filepath = "test_cache.yini";
    std::string ymeta_filepath = "test_cache.ymeta";

    // 1. Create a .yini and a .ymeta file
    {
        std::ofstream yini_file(yini_filepath);
        yini_file << "[Cache]\nvalue = 1";
    }
    {
        nlohmann::json j;
        j["Cache"]["value"] = 1;
        std::ofstream ymeta_file(ymeta_filepath);
        ymeta_file << j.dump();
    }

    // Ensure ymeta is newer (sleep for a bit to ensure timestamp difference)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
        std::ofstream yini_file(yini_filepath, std::ios_base::app);
        yini_file << "\n";
    }
     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
        std::ofstream ymeta_file(ymeta_filepath, std::ios_base::app);
        ymeta_file << "\n";
    }


    // 2. Test loading from cache
    YmetaManager ymeta_manager;
    auto cached_config = ymeta_manager.read(yini_filepath);
    ASSERT_TRUE(cached_config.has_value());
    EXPECT_EQ(std::get<int>((*cached_config)["Cache"]["value"]), 1);

    // 3. Test cache invalidation
    // Make the .yini file newer than the .ymeta file
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
        std::ofstream yini_file(yini_filepath, std::ios_base::app);
        yini_file << "// updated";
    }

    auto stale_config = ymeta_manager.read(yini_filepath);
    EXPECT_FALSE(stale_config.has_value());

    // 4. Teardown
    std::remove(yini_filepath.c_str());
    std::remove(ymeta_filepath.c_str());
}

TEST(YmetaTest, YiniWriteBack) {
    // 1. Setup: Create a config object
    Config config;
    auto& section = config["Test"];
    section["my_string"] = "hello";
    section["my_int"] = 123;
    auto arr = std::make_unique<Array>();
    arr->elements.push_back(1);
    arr->elements.push_back("two");
    section["my_array"] = std::move(arr);

    // 2. Action: Write the config to a .yini file
    std::string yini_filepath = "test_write_back.yini";
    YmetaManager ymeta_manager;
    ymeta_manager.write_yini(yini_filepath, config);

    // 3. Verification: Read the file and check its content
    std::ifstream yini_file(yini_filepath);
    std::stringstream buffer;
    buffer << yini_file.rdbuf();
    std::string content = buffer.str();

    std::string expected_content =
        "[Test]\n"
        "my_array = [1, \"two\"]\n"
        "my_int = 123\n"
        "my_string = \"hello\"\n\n";

    EXPECT_EQ(content, expected_content);

    // 4. Teardown
    std::remove(yini_filepath.c_str());
}