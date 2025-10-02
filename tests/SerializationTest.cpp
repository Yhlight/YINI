#include <gtest/gtest.h>
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Deserializer.h"
#include "Core/YiniValue.h"
#include <map>
#include <string>
#include <vector>
#include <variant>

TEST(SerializationTest, SerializesAndDeserializesData)
{
    // 1. Create complex data structure
    std::map<std::string, std::map<std::string, YINI::YiniValue>> original_data;
    YINI::YiniArray my_array = {YINI::YiniValue(1.0), YINI::YiniValue(std::string("two")), YINI::YiniValue(true)};
    YINI::YiniMap my_map = {{"a", YINI::YiniValue(1.0)}, {"b", YINI::YiniValue(false)}};

    original_data["Section1"]["key1"] = YINI::YiniValue(std::string("value1"));
    original_data["Section1"]["key2"] = YINI::YiniValue(123.0);
    original_data["Section2"]["array"] = YINI::YiniValue(my_array);
    original_data["Section2"]["map"] = YINI::YiniValue(my_map);

    const std::string filepath = "test_serialization.ymeta";

    // 2. Serialize the data
    YINI::Serialization::Serializer serializer;
    ASSERT_NO_THROW(serializer.serialize(original_data, filepath));

    // 3. Deserialize the data
    YINI::Serialization::Deserializer deserializer;
    std::map<std::string, std::map<std::string, YINI::YiniValue>> deserialized_data;
    ASSERT_NO_THROW(deserialized_data = deserializer.deserialize(filepath));

    // 4. Compare the original and deserialized data
    ASSERT_EQ(original_data.size(), deserialized_data.size());

    // Check Section1
    const auto& s1_orig = original_data["Section1"];
    const auto& s1_deser = deserialized_data["Section1"];
    EXPECT_EQ(std::get<std::string>(s1_orig.at("key1").m_value), std::get<std::string>(s1_deser.at("key1").m_value));
    EXPECT_EQ(std::get<double>(s1_orig.at("key2").m_value), std::get<double>(s1_deser.at("key2").m_value));

    // Check Section2 Array
    const auto& arr_orig_val = original_data.at("Section2").at("array");
    const auto& arr_deser_val = deserialized_data.at("Section2").at("array");
    const auto& arr_orig = *std::get<std::unique_ptr<YINI::YiniArray>>(arr_orig_val.m_value);
    const auto& arr_deser = *std::get<std::unique_ptr<YINI::YiniArray>>(arr_deser_val.m_value);

    ASSERT_EQ(arr_orig.size(), arr_deser.size());
    EXPECT_EQ(std::get<double>(arr_orig[0].m_value), std::get<double>(arr_deser[0].m_value));
    EXPECT_EQ(std::get<std::string>(arr_orig[1].m_value), std::get<std::string>(arr_deser[1].m_value));
    EXPECT_EQ(std::get<bool>(arr_orig[2].m_value), std::get<bool>(arr_deser[2].m_value));

    // Check Section2 Map
    const auto& map_orig_val = original_data.at("Section2").at("map");
    const auto& map_deser_val = deserialized_data.at("Section2").at("map");
    const auto& map_orig = *std::get<std::unique_ptr<YINI::YiniMap>>(map_orig_val.m_value);
    const auto& map_deser = *std::get<std::unique_ptr<YINI::YiniMap>>(map_deser_val.m_value);

    ASSERT_EQ(map_orig.size(), map_deser.size());
    EXPECT_EQ(std::get<double>(map_orig.at("a").m_value), std::get<double>(map_deser.at("a").m_value));
    EXPECT_EQ(std::get<bool>(map_orig.at("b").m_value), std::get<bool>(map_deser.at("b").m_value));
}