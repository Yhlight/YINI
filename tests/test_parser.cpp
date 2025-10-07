#include "gtest/gtest.h"
#include "YiniParser.h"

TEST(YiniParserTest, ParsesInteger) {
    YiniParser parser;
    parser.parse("[test]\nkey = 123\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<int>());
    EXPECT_EQ(value->get<int>(), 123);
}

TEST(YiniParserTest, ParsesFloat) {
    YiniParser parser;
    parser.parse("[test]\nkey = 3.14\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<double>());
    EXPECT_DOUBLE_EQ(value->get<double>(), 3.14);
}

TEST(YiniParserTest, ParsesBooleanTrue) {
    YiniParser parser;
    parser.parse("[test]\nkey = true\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<bool>());
    EXPECT_EQ(value->get<bool>(), true);
}

TEST(YiniParserTest, ParsesBooleanFalse) {
    YiniParser parser;
    parser.parse("[test]\nkey = false\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<bool>());
    EXPECT_EQ(value->get<bool>(), false);
}

TEST(YiniParserTest, ParsesString) {
    YiniParser parser;
    parser.parse("[test]\nkey = \"hello world\"\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<std::string>());
    EXPECT_EQ(value->get<std::string>(), "hello world");
}

TEST(YiniParserTest, ParsesArray) {
    YiniParser parser;
    parser.parse("[test]\nkey = [1, \"two\", true]\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<YiniArray>());
    const auto& arr = value->get<YiniArray>();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get<int>(), 1);
    EXPECT_EQ(arr[1].get<std::string>(), "two");
    EXPECT_EQ(arr[2].get<bool>(), true);
}

TEST(YiniParserTest, ParsesMap) {
    YiniParser parser;
    parser.parse("[test]\nkey = { a: 1, b: \"two\", c: false }\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<YiniMap>());
    const auto& map = value->get<YiniMap>();
    ASSERT_EQ(map.size(), 3);
    EXPECT_EQ(map.at("a").get<int>(), 1);
    EXPECT_EQ(map.at("b").get<std::string>(), "two");
    EXPECT_EQ(map.at("c").get<bool>(), false);
}

TEST(YiniParserTest, ParsesNestedContainers) {
    YiniParser parser;
    parser.parse("[test]\nkey = [{ a: 1 }, [2, 3]]\n");
    auto value = parser.getValue("test", "key");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<YiniArray>());
    const auto& arr = value->get<YiniArray>();
    ASSERT_EQ(arr.size(), 2);
    ASSERT_TRUE(arr[0].is<YiniMap>());
    const auto& map = arr[0].get<YiniMap>();
    EXPECT_EQ(map.at("a").get<int>(), 1);
    ASSERT_TRUE(arr[1].is<YiniArray>());
    const auto& nestedArr = arr[1].get<YiniArray>();
    EXPECT_EQ(nestedArr[0].get<int>(), 2);
    EXPECT_EQ(nestedArr[1].get<int>(), 3);
}

TEST(YiniParserTest, HandlesSectionInheritance) {
    YiniParser parser;
    parser.parse(
        "[parent]\n"
        "key1 = 1\n"
        "key2 = 2\n"
        "[child : parent]\n"
        "key3 = 3\n"
    );
    auto value1 = parser.getValue("child", "key1");
    auto value2 = parser.getValue("child", "key2");
    auto value3 = parser.getValue("child", "key3");
    ASSERT_TRUE(value1.has_value());
    ASSERT_TRUE(value2.has_value());
    ASSERT_TRUE(value3.has_value());
    EXPECT_EQ(value1->get<int>(), 1);
    EXPECT_EQ(value2->get<int>(), 2);
    EXPECT_EQ(value3->get<int>(), 3);
}

TEST(YiniParserTest, HandlesInheritanceOverride) {
    YiniParser parser;
    parser.parse(
        "[parent]\n"
        "key1 = 1\n"
        "[child : parent]\n"
        "key1 = 100\n"
    );
    auto value = parser.getValue("child", "key1");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->get<int>(), 100);
}

TEST(YiniParserTest, HandlesMacros) {
    YiniParser parser;
    parser.parse(
        "[#define]\n"
        "name = \"YINI\"\n"
        "[test]\n"
        "greeting = @name\n"
    );
    auto value = parser.getValue("test", "greeting");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<std::string>());
    EXPECT_EQ(value->get<std::string>(), "YINI");
}

TEST(YiniParserTest, HandlesCrossSectionReferences) {
    YiniParser parser;
    parser.parse(
        "[source]\n"
        "value = 42\n"
        "[target]\n"
        "ref = @{source.value}\n"
    );
    auto value = parser.getValue("target", "ref");
    ASSERT_TRUE(value.has_value());
    ASSERT_TRUE(value->is<int>());
    EXPECT_EQ(value->get<int>(), 42);
}