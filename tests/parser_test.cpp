#include "YiniParser.h"
#include "YiniValue.h"
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
    EXPECT_EQ(value->get<std::string>(), "value");
}

TEST(ParserTest, MacroDefinitionAndReference)
{
    YiniParser parser;
    std::string content =
        "[#define]\n"
        "color = \"blue\"\n"
        "[UI]\n"
        "button_color = @color\n";
    parser.parse(content);

    auto value = parser.getValue("UI", "button_color");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<std::string>(), "blue");
}

TEST(ParserTest, CrossSectionReference)
{
    YiniParser parser;
    std::string content =
        "[Config]\n"
        "target_width = 1920\n"
        "[Window]\n"
        "width = @{Config.target_width}\n";
    parser.parse(content);

    auto value = parser.getValue("Window", "width");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<int>());
    EXPECT_EQ(value->get<int>(), 1920);
}

TEST(ParserTest, ParseBasicArray)
{
    YiniParser parser;
    std::string content = "[Data]\narray = [1, \"two\", true]";
    parser.parse(content);

    auto value = parser.getValue("Data", "array");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<YiniArray>());

    const auto& arr = value->get<YiniArray>();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get<int>(), 1);
    EXPECT_EQ(arr[1].get<std::string>(), "two");
    EXPECT_EQ(arr[2].get<bool>(), true);
}

TEST(ParserTest, ParseBasicMap)
{
    YiniParser parser;
    std::string content = "[Data]\nmap = { name: \"Jules\", age: 42 }";
    parser.parse(content);

    auto value = parser.getValue("Data", "map");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<YiniMap>());

    const auto& map = value->get<YiniMap>();
    ASSERT_EQ(map.size(), 2);

    auto name_it = map.find("name");
    ASSERT_NE(name_it, map.end());
    EXPECT_EQ(name_it->second.get<std::string>(), "Jules");

    auto age_it = map.find("age");
    ASSERT_NE(age_it, map.end());
    EXPECT_EQ(age_it->second.get<int>(), 42);
}

TEST(ParserTest, SingleInheritance)
{
    YiniParser parser;
    std::string content = "[Parent]\nkey = parent_value\n[Child : Parent]\n";
    parser.parse(content);

    auto value = parser.getValue("Child", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<std::string>(), "parent_value");
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
    EXPECT_EQ(val1->get<std::string>(), "p1_val");

    auto val2 = parser.getValue("Child", "key2");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2->get<std::string>(), "p2_val");
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
    EXPECT_EQ(value->get<std::string>(), "child_value");
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
    EXPECT_EQ(value->get<std::string>(), "p2_val"); // Parent2 should override Parent1
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
    EXPECT_EQ(val1->get<std::string>(), "value1");

    auto val2 = parser.getValue("MyList", "1");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2->get<std::string>(), "value2");

    auto val3 = parser.getValue("MyList", "2");
    ASSERT_TRUE(val3.has_value());
    EXPECT_EQ(val3->get<int>(), 123);
}

TEST(ParserTest, ParseInteger)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = 123";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<int>(), 123);
}

TEST(ParserTest, ParseBooleanTrue)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = true";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<bool>(), true);
}

TEST(ParserTest, ParseBooleanFalse)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = false";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<bool>(), false);
}

TEST(ParserTest, ParseDouble)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = 3.14";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<double>(), 3.14);
}

TEST(ParserTest, IgnoresSingleLineComment)
{
    YiniParser parser;
    std::string content = "[Section]\n// this is a comment\nkey = value";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<std::string>(), "value");
    EXPECT_FALSE(parser.getValue("Section", "// this is a comment").has_value());
}

TEST(ParserTest, IgnoresMultiLineComment)
{
    YiniParser parser;
    std::string content = "[Section]\n/* this is a \n multi-line comment */\nkey = value";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<std::string>(), "value");
}

TEST(ParserTest, KeyValueFollowedByComment)
{
    YiniParser parser;
    std::string content = "[Section]\nkey = value // this is a comment";
    parser.parse(content);

    auto value = parser.getValue("Section", "key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<std::string>(), "value");
}