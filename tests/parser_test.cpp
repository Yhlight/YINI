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

TEST(ParserTest, SingleInheritance)
{
    YiniParser parser;
    std::string content = "[Parent]\nkey = parent_value\n[Child : Parent]\n";
    parser.parse(content);

    auto value = parser.getValue("Child", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "parent_value");
}

TEST(ParserTest, MultipleInheritance)
{
    YiniParser parser;
    std::string content =
        "[Parent1]\nkey1 = p1_val\n"
        "[Parent2]\nkey2 = p2_val\n"
        "[Child : Parent1, Parent2]\n";
    parser.parse(content);

    auto val1 = parser.getValue("Child", "key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(std::get<std::string>(val1.value()), "p1_val");

    auto val2 = parser.getValue("Child", "key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(std::get<std::string>(val2.value()), "p2_val");
}

TEST(ParserTest, OverrideInheritedValue)
{
    YiniParser parser;
    std::string content =
        "[Parent]\nkey = parent_value\n"
        "[Child : Parent]\nkey = child_value";
    parser.parse(content);

    auto value = parser.getValue("Child", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "child_value");
}

TEST(ParserTest, MultipleInheritanceOverrideOrder)
{
    YiniParser parser;
    std::string content =
        "[Parent1]\nkey = p1_val\n"
        "[Parent2]\nkey = p2_val\n"
        "[Child : Parent1, Parent2]\n";
    parser.parse(content);

    auto value = parser.getValue("Child", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "p2_val"); // Parent2 should override Parent1
}

TEST(ParserTest, QuickValueRegistration)
{
    YiniParser parser;
    std::string content =
        "[MyList]\n"
        "+= value1\n"
        "+= value2\n"
        "+= 123\n";
    parser.parse(content);

    auto val1 = parser.getValue("MyList", "0");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(std::get<std::string>(val1.value()), "value1");

    auto val2 = parser.getValue("MyList", "1");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(std::get<std::string>(val2.value()), "value2");

    auto val3 = parser.getValue("MyList", "2");
    ASSERT_TRUE(val3.has_value());
    EXPECT_EQ(std::get<int>(val3.value()), 123);
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

TEST(ParserTest, IgnoresSingleLineComment)
{
    YiniParser parser;
    std::string content = "[Section]\n// this is a comment\nkey = value";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "value");
    EXPECT_FALSE(parser.getValue("Section", "// this is a comment").has_value());
}

TEST(ParserTest, IgnoresMultiLineComment)
{
    YiniParser parser;
    std::string content = "[Section]\n/* this is a \n multi-line comment */\nkey = value";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "value");
}

TEST(ParserTest, KeyValueFollowedByComment)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = value // this is a comment";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<std::string>(value.value()), "value");
}