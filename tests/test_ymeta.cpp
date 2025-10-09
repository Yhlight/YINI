#include <gtest/gtest.h>
#include "Ymeta/YmetaManager.h"
#include "Parser/parser.h"
#include <fstream>
#include <nlohmann/json.hpp>

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