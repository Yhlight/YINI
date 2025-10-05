#include <gtest/gtest.h>

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "Core/Serialization/Deserializer.h"
#include "Core/Serialization/Serializer.h"
#include "Core/YiniValue.h"

TEST(SerializationTest, SerializesAndDeserializesData) {
    // 1. Create complex data structure
    std::map<std::string, std::map<std::string, YINI::YiniValue, std::less<>>, std::less<>> original_data;
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
    std::map<std::string, std::map<std::string, YINI::YiniValue, std::less<>>, std::less<>> deserialized_data;
    ASSERT_NO_THROW(deserialized_data = deserializer.deserialize(filepath));

    // 4. Compare the original and deserialized data
    ASSERT_EQ(original_data.size(), deserialized_data.size());

    // Check Section1
    const auto& s1_orig = original_data.find("Section1")->second;
    const auto& s1_deser = deserialized_data.find("Section1")->second;
    EXPECT_EQ(std::get<std::string>(s1_orig.find("key1")->second.m_value),
              std::get<std::string>(s1_deser.find("key1")->second.m_value));
    EXPECT_EQ(std::get<double>(s1_orig.find("key2")->second.m_value),
              std::get<double>(s1_deser.find("key2")->second.m_value));

    // Check Section2 Array
    const auto& arr_orig_val = original_data.find("Section2")->second.find("array")->second;
    const auto& arr_deser_val = deserialized_data.find("Section2")->second.find("array")->second;
    const auto& arr_orig = *std::get<std::unique_ptr<YINI::YiniArray>>(arr_orig_val.m_value);
    const auto& arr_deser = *std::get<std::unique_ptr<YINI::YiniArray>>(arr_deser_val.m_value);

    ASSERT_EQ(arr_orig.size(), arr_deser.size());
    EXPECT_EQ(std::get<double>(arr_orig[0].m_value), std::get<double>(arr_deser[0].m_value));
    EXPECT_EQ(std::get<std::string>(arr_orig[1].m_value), std::get<std::string>(arr_deser[1].m_value));
    EXPECT_EQ(std::get<bool>(arr_orig[2].m_value), std::get<bool>(arr_deser[2].m_value));

    // Check Section2 Map
    const auto& map_orig_val = original_data.find("Section2")->second.find("map")->second;
    const auto& map_deser_val = deserialized_data.find("Section2")->second.find("map")->second;
    const auto& map_orig = *std::get<std::unique_ptr<YINI::YiniMap>>(map_orig_val.m_value);
    const auto& map_deser = *std::get<std::unique_ptr<YINI::YiniMap>>(map_deser_val.m_value);

    ASSERT_EQ(map_orig.size(), map_deser.size());
    EXPECT_EQ(std::get<double>(map_orig.find("a")->second.m_value),
              std::get<double>(map_deser.find("a")->second.m_value));
    EXPECT_EQ(std::get<bool>(map_orig.find("b")->second.m_value), std::get<bool>(map_deser.find("b")->second.m_value));
}