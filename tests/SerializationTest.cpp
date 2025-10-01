#include <gtest/gtest.h>
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Deserializer.h"
#include <map>
#include <string>
#include <any>
#include <vector>

TEST(SerializationTest, SerializesAndDeserializesData)
{
    // 1. Create complex data structure
    std::map<std::string, std::map<std::string, std::any>> original_data;
    std::vector<std::any> my_array = {1.0, std::string("two"), true};
    std::map<std::string, std::any> my_map = {{"a", 1.0}, {"b", false}};

    original_data["Section1"]["key1"] = std::string("value1");
    original_data["Section1"]["key2"] = 123.0;
    original_data["Section2"]["array"] = my_array;
    original_data["Section2"]["map"] = my_map;

    const std::string filepath = "test_serialization.ymeta";

    // 2. Serialize the data
    YINI::Serialization::Serializer serializer;
    ASSERT_NO_THROW(serializer.serialize(original_data, filepath));

    // 3. Deserialize the data
    YINI::Serialization::Deserializer deserializer;
    std::map<std::string, std::map<std::string, std::any>> deserialized_data;
    ASSERT_NO_THROW(deserialized_data = deserializer.deserialize(filepath));

    // 4. Compare the original and deserialized data
    ASSERT_EQ(original_data.size(), deserialized_data.size());

    // Check Section1
    const auto& s1_orig = original_data["Section1"];
    const auto& s1_deser = deserialized_data["Section1"];
    EXPECT_EQ(std::any_cast<std::string>(s1_orig.at("key1")), std::any_cast<std::string>(s1_deser.at("key1")));
    EXPECT_EQ(std::any_cast<double>(s1_orig.at("key2")), std::any_cast<double>(s1_deser.at("key2")));

    // Check Section2 Array
    const auto& arr_orig = std::any_cast<const std::vector<std::any>&>(original_data.at("Section2").at("array"));
    const auto& arr_deser = std::any_cast<const std::vector<std::any>&>(deserialized_data.at("Section2").at("array"));
    ASSERT_EQ(arr_orig.size(), arr_deser.size());
    EXPECT_EQ(std::any_cast<double>(arr_orig[0]), std::any_cast<double>(arr_deser[0]));
    EXPECT_EQ(std::any_cast<std::string>(arr_orig[1]), std::any_cast<std::string>(arr_deser[1]));
    EXPECT_EQ(std::any_cast<bool>(arr_orig[2]), std::any_cast<bool>(arr_deser[2]));

    // Check Section2 Map
    const auto& map_orig = std::any_cast<const std::map<std::string, std::any>&>(original_data.at("Section2").at("map"));
    const auto& map_deser = std::any_cast<const std::map<std::string, std::any>&>(deserialized_data.at("Section2").at("map"));
    ASSERT_EQ(map_orig.size(), map_deser.size());
    EXPECT_EQ(std::any_cast<double>(map_orig.at("a")), std::any_cast<double>(map_deser.at("a")));
    EXPECT_EQ(std::any_cast<bool>(map_orig.at("b")), std::any_cast<bool>(map_deser.at("b")));
}