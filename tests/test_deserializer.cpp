#include <gtest/gtest.h>
#include "YINI/JsonDeserializer.hpp"
#include "YINI/YiniData.hpp"

TEST(JsonDeserializerTest, DeserializeDocument)
{
    const std::string json_input = R"({
        "Core":{
            "name":"YINI",
            "version":1.0,
            "enabled":true,
            "data":[1, "two", false]
        }
    })";

    YINI::YiniDocument doc;
    bool success = YINI::JsonDeserializer::deserialize(json_input, doc);

    ASSERT_TRUE(success);
    ASSERT_EQ(doc.getSections().size(), 1);

    const auto *section = doc.findSection("Core");
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->pairs.size(), 4);

    auto name_it =
        std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto &p) { return p.key == "name"; });
    ASSERT_NE(name_it, section->pairs.end());
    EXPECT_EQ(std::get<std::string>(name_it->value.data), "YINI");

    auto version_it =
        std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto &p) { return p.key == "version"; });
    ASSERT_NE(version_it, section->pairs.end());
    EXPECT_EQ(std::get<double>(version_it->value.data), 1.0);

    auto enabled_it =
        std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto &p) { return p.key == "enabled"; });
    ASSERT_NE(enabled_it, section->pairs.end());
    EXPECT_EQ(std::get<bool>(enabled_it->value.data), true);

    auto data_it =
        std::find_if(section->pairs.begin(), section->pairs.end(), [](const auto &p) { return p.key == "data"; });
    ASSERT_NE(data_it, section->pairs.end());
    auto &arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(data_it->value.data);
    ASSERT_NE(arr_ptr, nullptr);
    ASSERT_EQ(arr_ptr->elements.size(), 3);
    EXPECT_EQ(std::get<int>(arr_ptr->elements[0].data), 1);
    EXPECT_EQ(std::get<std::string>(arr_ptr->elements[1].data), "two");
    EXPECT_EQ(std::get<bool>(arr_ptr->elements[2].data), false);
}