#include <gtest/gtest.h>
#include "YINI/YiniData.hpp"
#include "YINI/Parser.hpp"

TEST(ParserTest, ParseSimpleSection)
{
    const std::string input = "[TestSection]\nkey = \"value\"";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    EXPECT_EQ(section.name, "TestSection");
    ASSERT_EQ(section.pairs.size(), 1);
    const auto& pair = section.pairs[0];
    EXPECT_EQ(pair.key, "key");
    EXPECT_EQ(std::get<std::string>(pair.value.data), "value");
}

TEST(ParserTest, ParseValueTypes)
{
    const std::string input =
        "[DataTypes]\n"
        "integer = 123\n"
        "float = 3.14\n"
        "boolean_true = true\n"
        "boolean_false = false\n";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    EXPECT_EQ(section.name, "DataTypes");
    ASSERT_EQ(section.pairs.size(), 4);

    const auto& int_pair = section.pairs[0];
    EXPECT_EQ(int_pair.key, "integer");
    EXPECT_EQ(std::get<int>(int_pair.value.data), 123);

    const auto& float_pair = section.pairs[1];
    EXPECT_EQ(float_pair.key, "float");
    EXPECT_EQ(std::get<double>(float_pair.value.data), 3.14);

    const auto& bool_true_pair = section.pairs[2];
    EXPECT_EQ(bool_true_pair.key, "boolean_true");
    EXPECT_EQ(std::get<bool>(bool_true_pair.value.data), true);

    const auto& bool_false_pair = section.pairs[3];
    EXPECT_EQ(bool_false_pair.key, "boolean_false");
    EXPECT_EQ(std::get<bool>(bool_false_pair.value.data), false);
}

TEST(ParserTest, ParseArrayValue)
{
    const std::string input =
        "[Arrays]\n"
        "int_array = [1, 2, 3]\n";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    ASSERT_EQ(section.pairs.size(), 1);
    const auto& pair = section.pairs[0];
    EXPECT_EQ(pair.key, "int_array");

    auto& arr_ptr = std::get<std::unique_ptr<YINI::YiniArray>>(pair.value.data);
    ASSERT_NE(arr_ptr, nullptr);
    auto& arr = arr_ptr->elements;
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(std::get<int>(arr[0].data), 1);
    EXPECT_EQ(std::get<int>(arr[1].data), 2);
    EXPECT_EQ(std::get<int>(arr[2].data), 3);
}

TEST(ParserTest, ParseSectionInheritance)
{
    const std::string input = "[Derived] : Base1, Base2";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    EXPECT_EQ(section.name, "Derived");
    ASSERT_EQ(section.inheritedSections.size(), 2);
    EXPECT_EQ(section.inheritedSections[0], "Base1");
    EXPECT_EQ(section.inheritedSections[1], "Base2");
}

TEST(ParserTest, ParseQuickRegistration)
{
    const std::string input =
        "[Registry]\n"
        "+= 1\n"
        "+= \"two\"\n"
        "+= true\n";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    EXPECT_EQ(section.name, "Registry");
    ASSERT_EQ(section.registrationList.size(), 3);
    EXPECT_EQ(std::get<int>(section.registrationList[0].data), 1);
    EXPECT_EQ(std::get<std::string>(section.registrationList[1].data), "two");
    EXPECT_EQ(std::get<bool>(section.registrationList[2].data), true);
}

TEST(ParserTest, ParseSectionWithComments)
{
    const std::string input =
        "// This is a whole line comment\n"
        "[TestSection] // This is an inline comment\n"
        "key = \"value\"\n";
    YINI::Parser parser(input);
    YINI::YiniDocument doc = parser.parse();

    ASSERT_EQ(doc.getSections().size(), 1);
    const auto& section = doc.getSections()[0];
    EXPECT_EQ(section.name, "TestSection");
    ASSERT_EQ(section.pairs.size(), 1);
    const auto& pair = section.pairs[0];
    EXPECT_EQ(pair.key, "key");
    EXPECT_EQ(std::get<std::string>(pair.value.data), "value");
}