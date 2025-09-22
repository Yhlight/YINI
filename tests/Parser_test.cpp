#include <gtest/gtest.h>
#include "Parser.h"
#include "Lexer.h"

TEST(ParserTest, BasicParsing)
{
    std::string input = R"yini(
[TestSection]
key1 = "value1"
key2 = 123
key3 = 3.14
key4 = true
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    ASSERT_EQ(parser.getErrors().size(), 0);

    auto section = data.getSection("TestSection");
    ASSERT_NE(section, nullptr);

    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 4);

    // key1
    auto it1 = kvs.find("key1");
    ASSERT_NE(it1, kvs.end());
    ASSERT_NO_THROW(it1->second.get<Yini::YiniString>());
    EXPECT_EQ(it1->second.get<Yini::YiniString>(), "value1");

    // key2
    auto it2 = kvs.find("key2");
    ASSERT_NE(it2, kvs.end());
    ASSERT_NO_THROW(it2->second.get<Yini::YiniInteger>());
    EXPECT_EQ(it2->second.get<Yini::YiniInteger>(), 123);

    // key3
    auto it3 = kvs.find("key3");
    ASSERT_NE(it3, kvs.end());
    ASSERT_NO_THROW(it3->second.get<Yini::YiniFloat>());
    EXPECT_EQ(it3->second.get<Yini::YiniFloat>(), 3.14);

    // key4
    auto it4 = kvs.find("key4");
    ASSERT_NE(it4, kvs.end());
    ASSERT_NO_THROW(it4->second.get<Yini::YiniBoolean>());
    EXPECT_EQ(it4->second.get<Yini::YiniBoolean>(), true);
}

TEST(ParserTest, InheritanceAndPlusEqual)
{
    std::string input = R"yini(
[Base]
base_key = "base_value"

[Derived] : Base
derived_key = 123
+= "quick"
+= 456
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto derived_section = data.getSection("Derived");
    ASSERT_NE(derived_section, nullptr);

    // Check inheritance
    auto inheritance = derived_section->getInheritance();
    ASSERT_EQ(inheritance.size(), 1);
    EXPECT_EQ(inheritance[0], "Base");

    // Check key-values
    auto kvs = derived_section->getKeyValues();
    ASSERT_EQ(kvs.size(), 1);
    EXPECT_EQ(kvs["derived_key"].get<Yini::YiniInteger>(), 123);

    // Check += values
    auto values = derived_section->getValues();
    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(values[0].get<Yini::YiniString>(), "quick");
    EXPECT_EQ(values[1].get<Yini::YiniInteger>(), 456);
}

TEST(ParserTest, MultipleInheritance)
{
    std::string input = R"yini(
[Base1]
[Base2]
[Derived] : Base1, Base2
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto derived_section = data.getSection("Derived");
    ASSERT_NE(derived_section, nullptr);

    auto inheritance = derived_section->getInheritance();
    ASSERT_EQ(inheritance.size(), 2);
    EXPECT_EQ(inheritance[0], "Base1");
    EXPECT_EQ(inheritance[1], "Base2");
}

TEST(ParserTest, ArrayParsing)
{
    std::string input = R"yini(
[Arrays]
arr1 = [1, 2, 3]
arr2 = ["a", "b", "c"]
arr3 = [1, "two", 3.0, true]
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto section = data.getSection("Arrays");
    ASSERT_NE(section, nullptr);

    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 3);

    // arr1
    auto it1 = kvs.find("arr1");
    ASSERT_NE(it1, kvs.end());
    auto& arr1 = it1->second.get<Yini::YiniArray>();
    ASSERT_EQ(arr1.size(), 3);
    EXPECT_EQ(arr1[0].get<Yini::YiniInteger>(), 1);
    EXPECT_EQ(arr1[1].get<Yini::YiniInteger>(), 2);
    EXPECT_EQ(arr1[2].get<Yini::YiniInteger>(), 3);

    // arr2
    auto it2 = kvs.find("arr2");
    ASSERT_NE(it2, kvs.end());
    auto& arr2 = it2->second.get<Yini::YiniArray>();
    ASSERT_EQ(arr2.size(), 3);
    EXPECT_EQ(arr2[0].get<Yini::YiniString>(), "a");
    EXPECT_EQ(arr2[1].get<Yini::YiniString>(), "b");
    EXPECT_EQ(arr2[2].get<Yini::YiniString>(), "c");

    // arr3 (mixed types)
    auto it3 = kvs.find("arr3");
    ASSERT_NE(it3, kvs.end());
    auto& arr3 = it3->second.get<Yini::YiniArray>();
    ASSERT_EQ(arr3.size(), 4);
    EXPECT_EQ(arr3[0].get<Yini::YiniInteger>(), 1);
    EXPECT_EQ(arr3[1].get<Yini::YiniString>(), "two");
    EXPECT_EQ(arr3[2].get<Yini::YiniFloat>(), 3.0);
    EXPECT_EQ(arr3[3].get<Yini::YiniBoolean>(), true);
}

TEST(ParserTest, CoordinateParsing)
{
    std::string input = R"yini(
[Coordinates]
pos2d = (10, 20)
pos3d = (1.5, 2.5, 3.5)
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto section = data.getSection("Coordinates");
    ASSERT_NE(section, nullptr);

    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 2);

    // pos2d
    auto it1 = kvs.find("pos2d");
    ASSERT_NE(it1, kvs.end());
    auto& pos2d = it1->second.get<Yini::Coordinate2D>();
    EXPECT_EQ(pos2d.x, 10);
    EXPECT_EQ(pos2d.y, 20);

    // pos3d
    auto it2 = kvs.find("pos3d");
    ASSERT_NE(it2, kvs.end());
    auto& pos3d = it2->second.get<Yini::Coordinate3D>();
    EXPECT_FLOAT_EQ(pos3d.x, 1.5);
    EXPECT_FLOAT_EQ(pos3d.y, 2.5);
    EXPECT_FLOAT_EQ(pos3d.z, 3.5);
}

TEST(ParserTest, MapParsing)
{
    std::string input = R"yini(
[Maps]
map1 = {
    key1: "value1",
    key2: 123,
    key3: { nested_key: "nested_value" }
}
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto section = data.getSection("Maps");
    ASSERT_NE(section, nullptr);

    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 1);

    auto it = kvs.find("map1");
    ASSERT_NE(it, kvs.end());
    auto& map1 = it->second.get<Yini::YiniMap>();
    ASSERT_EQ(map1.size(), 3);

    EXPECT_EQ(map1["key1"].get<Yini::YiniString>(), "value1");
    EXPECT_EQ(map1["key2"].get<Yini::YiniInteger>(), 123);

    auto& nested_map = map1["key3"].get<Yini::YiniMap>();
    ASSERT_EQ(nested_map.size(), 1);
    EXPECT_EQ(nested_map["nested_key"].get<Yini::YiniString>(), "nested_value");
}

TEST(ParserTest, ColorParsing)
{
    std::string input = R"yini(
[Colors]
color1 = #FF0000
color4 = Color(255, 192, 203)
color5 = color(128, 128, 128)
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto section = data.getSection("Colors");
    ASSERT_NE(section, nullptr);

    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 3);

    // color1
    auto it1 = kvs.find("color1");
    ASSERT_NE(it1, kvs.end());
    auto& color1 = it1->second.get<Yini::ColorRGB>();
    EXPECT_EQ(color1.r, 255);
    EXPECT_EQ(color1.g, 0);
    EXPECT_EQ(color1.b, 0);

    // color4
    auto it4 = kvs.find("color4");
    ASSERT_NE(it4, kvs.end());
    auto& color4 = it4->second.get<Yini::ColorRGB>();
    EXPECT_EQ(color4.r, 255);
    EXPECT_EQ(color4.g, 192);
    EXPECT_EQ(color4.b, 203);

    // color5
    auto it5 = kvs.find("color5");
    ASSERT_NE(it5, kvs.end());
    auto& color5 = it5->second.get<Yini::ColorRGB>();
    EXPECT_EQ(color5.r, 128);
    EXPECT_EQ(color5.g, 128);
    EXPECT_EQ(color5.b, 128);
}

TEST(ParserTest, MacroParsing)
{
    std::string input = R"yini(
[#define]
greeting = "Hello"

[Test]
message = @greeting
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    // Check that macro was stored
    auto macros = data.getMacros();
    ASSERT_EQ(macros.size(), 1);
    EXPECT_EQ(macros["greeting"].get<Yini::YiniString>(), "Hello");

    // Check that macro was substituted
    auto section = data.getSection("Test");
    ASSERT_NE(section, nullptr);
    auto kvs = section->getKeyValues();
    ASSERT_EQ(kvs.size(), 1);
    EXPECT_EQ(kvs["message"].get<Yini::YiniString>(), "Hello");
}

TEST(ParserTest, IncludeParsing)
{
    std::string input = R"yini(
[#include]
+= "base.yini"
)yini";

    Yini::Lexer lexer(input);
    Yini::Parser parser(lexer);
    Yini::YiniData data = parser.parseYini();

    for(const auto& err : parser.getErrors())
    {
        std::cout << "Parser Error: " << err << std::endl;
    }
    ASSERT_EQ(parser.getErrors().size(), 0);

    auto includes = data.getIncludes();
    ASSERT_EQ(includes.size(), 1);
    EXPECT_EQ(includes[0], "base.yini");
}
