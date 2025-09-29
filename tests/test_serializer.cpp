#include <gtest/gtest.h>
#include "YINI/Parser.hpp"
#include "YINI/JsonSerializer.hpp"

TEST(JsonSerializerTest, SerializeDocument)
{
    const std::string input = R"([Core]
name = "YINI"
version = 1.0
enabled = true
data = [1, 2, 3]
)";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    parser.parse();

    std::string json_output = YINI::JsonSerializer::serialize(doc);

    // Basic checks for presence of key elements. A full JSON parser would be better,
    // but for a TDD driving test, this is sufficient to ensure serialization is happening.
    EXPECT_NE(json_output.find("\"Core\""), std::string::npos);
    EXPECT_NE(json_output.find("\"name\":\"YINI\""), std::string::npos);
    EXPECT_NE(json_output.find("\"version\":1.0"), std::string::npos);
    EXPECT_NE(json_output.find("\"enabled\":true"), std::string::npos);
    EXPECT_NE(json_output.find("\"data\":[1,2,3]"), std::string::npos);
}

TEST(JsonSerializerTest, SerializePairAndMap)
{
    const std::string input = R"([Data]
my_pair = {key: "value"}
my_map = {{a: 1, b: false}}
)";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    parser.parse();

    std::string json_output = YINI::JsonSerializer::serialize(doc);

    EXPECT_NE(json_output.find("\"my_pair\":{\"key\":\"value\"}"), std::string::npos);
    EXPECT_NE(json_output.find("\"my_map\":{\"a\":1,\"b\":false}"), std::string::npos);
}