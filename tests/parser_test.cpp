#include "YiniParser.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(ParserTest, ParseBasicSectionAndKeyValue)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = value";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "value");
}

TEST(ParserTest, ParseInteger)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = 123";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<int>(value.value()), 123);
}

TEST(ParserTest, ParseBooleanTrue)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = true";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<bool>(value.value()), true);
}

TEST(ParserTest, ParseBooleanFalse)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = false";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<bool>(value.value()), false);
}

TEST(ParserTest, ParseDouble)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = 3.14";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<double>(value.value()), 3.14);
}