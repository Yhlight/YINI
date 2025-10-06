#include <gtest/gtest.h>
#include "Parser.h"
#include "Value.h"

using namespace yini;

// Test fixture for Parser tests to reduce boilerplate
class ParserTest : public ::testing::Test {
protected:
    std::unique_ptr<Parser> createParser(const std::string& source) {
        auto parser = std::make_unique<Parser>(source);
        bool success = parser->parse();
        if (!success) {
            // Use GTest's failure reporting for better diagnostics
            ADD_FAILURE() << "Parsing failed: " << parser->getLastError();
        }
        EXPECT_TRUE(success);
        return parser;
    }
};

TEST_F(ParserTest, SimpleSection) {
    std::string source = R"(
[Config]
key1 = 123
key2 = "value"
key3 = true
    )";
    
    auto parser = createParser(source);
    const auto& sections = parser->getSections();
    ASSERT_NE(sections.find("Config"), sections.end());
    
    const auto& config = sections.at("Config");
    ASSERT_NE(config.entries.find("key1"), config.entries.end());
    ASSERT_TRUE(config.entries.at("key1")->isInteger());
    ASSERT_EQ(config.entries.at("key1")->asInteger(), 123);
    
    ASSERT_TRUE(config.entries.at("key2")->isString());
    ASSERT_EQ(config.entries.at("key2")->asString(), "value");
    
    ASSERT_TRUE(config.entries.at("key3")->isBoolean());
    ASSERT_EQ(config.entries.at("key3")->asBoolean(), true);
}

TEST_F(ParserTest, Arrays) {
    std::string source = R"(
[Config]
arr = [1, 2, 3]
    )";
    
    auto parser = createParser(source);
    const auto& config = parser->getSections().at("Config");
    
    ASSERT_TRUE(config.entries.at("arr")->isArray());
    auto arr = config.entries.at("arr")->asArray();
    ASSERT_EQ(arr.size(), 3);
    ASSERT_EQ(arr[0]->asInteger(), 1);
    ASSERT_EQ(arr[1]->asInteger(), 2);
    ASSERT_EQ(arr[2]->asInteger(), 3);
}

TEST_F(ParserTest, Inheritance) {
    std::string source = R"(
[Base]
key1 = 100
key2 = 200

[Derived] : Base
key2 = 300
key3 = 400
    )";
    
    auto parser = createParser(source);
    const auto& derived = parser->getSections().at("Derived");
    
    // key1 should be inherited
    ASSERT_NE(derived.entries.find("key1"), derived.entries.end());
    ASSERT_EQ(derived.entries.at("key1")->asInteger(), 100);
    
    // key2 should be overridden
    ASSERT_EQ(derived.entries.at("key2")->asInteger(), 300);
    
    // key3 is new
    ASSERT_EQ(derived.entries.at("key3")->asInteger(), 400);
}

TEST_F(ParserTest, QuickRegister) {
    std::string source = R"(
[Registry]
+= "value1"
+= "value2"
+= "value3"
    )";
    
    auto parser = createParser(source);
    const auto& registry = parser->getSections().at("Registry");
    
    ASSERT_EQ(registry.entries.size(), 3);
    ASSERT_EQ(registry.entries.at("0")->asString(), "value1");
    ASSERT_EQ(registry.entries.at("1")->asString(), "value2");
    ASSERT_EQ(registry.entries.at("2")->asString(), "value3");
}

TEST_F(ParserTest, Arithmetic) {
    std::string source = R"(
[Math]
add = 1 + 2
multiply = 3 * 4
complex = 1 + 2 * 3
    )";
    
    auto parser = createParser(source);
    const auto& math = parser->getSections().at("Math");
    
    ASSERT_EQ(math.entries.at("add")->asInteger(), 3);
    ASSERT_EQ(math.entries.at("multiply")->asInteger(), 12);
    ASSERT_EQ(math.entries.at("complex")->asInteger(), 7); // 1 + (2 * 3)
}

TEST_F(ParserTest, Color) {
    std::string source = R"(
[Visual]
color1 = #FF0000
color2 = Color(255, 0, 0)
    )";
    
    auto parser = createParser(source);
    const auto& visual = parser->getSections().at("Visual");
    
    ASSERT_TRUE(visual.entries.at("color1")->isColor());
    auto c1 = visual.entries.at("color1")->asColor();
    ASSERT_EQ(c1.r, 255);
    ASSERT_EQ(c1.g, 0);
    ASSERT_EQ(c1.b, 0);
    
    ASSERT_TRUE(visual.entries.at("color2")->isColor());
    auto c2 = visual.entries.at("color2")->asColor();
    ASSERT_EQ(c2.r, 255);
    ASSERT_EQ(c2.g, 0);
    ASSERT_EQ(c2.b, 0);
}

TEST_F(ParserTest, Coord) {
    std::string source = R"(
[Position]
pos2d = Coord(10, 20)
pos3d = Coord(10, 20, 30)
    )";
    
    auto parser = createParser(source);
    const auto& position = parser->getSections().at("Position");
    
    ASSERT_TRUE(position.entries.at("pos2d")->isCoord());
    auto c2d = position.entries.at("pos2d")->asCoord();
    ASSERT_EQ(c2d.x, 10);
    ASSERT_EQ(c2d.y, 20);
    ASSERT_FALSE(c2d.z.has_value());
    
    ASSERT_TRUE(position.entries.at("pos3d")->isCoord());
    auto c3d = position.entries.at("pos3d")->asCoord();
    ASSERT_EQ(c3d.x, 10);
    ASSERT_EQ(c3d.y, 20);
    ASSERT_TRUE(c3d.z.has_value());
    ASSERT_EQ(c3d.z.value(), 30);
}

TEST_F(ParserTest, DefinesAndReferences) {
    std::string source = R"(
[#define]
width = 1920
height = 1080

[Config]
key1 = @width
    )";
    
    auto parser = createParser(source);
    
    const auto& defines = parser->getDefines();
    ASSERT_NE(defines.find("width"), defines.end());
    ASSERT_EQ(defines.at("width")->asInteger(), 1920);
    
    // Check that macro reference was resolved
    const auto& config = parser->getSections().at("Config");
    ASSERT_NE(config.entries.find("key1"), config.entries.end());
    
    auto key1_value = config.entries.at("key1");
    ASSERT_TRUE(key1_value->isInteger());
    ASSERT_EQ(key1_value->asInteger(), 1920);
}

TEST_F(ParserTest, Includes) {
    std::string source = R"(
[#include]
+= "file1.yini"
+= "file2.yini"
    )";
    
    auto parser = createParser(source);
    const auto& includes = parser->getIncludes();
    ASSERT_EQ(includes.size(), 2);
    ASSERT_EQ(includes[0], "file1.yini");
    ASSERT_EQ(includes[1], "file2.yini");
}

TEST_F(ParserTest, Map) {
    std::string source = R"(
[Config]
settings = {width: 1920, height: 1080}
    )";
    
    auto parser = createParser(source);
    const auto& config = parser->getSections().at("Config");
    
    ASSERT_TRUE(config.entries.at("settings")->isMap());
    auto map = config.entries.at("settings")->asMap();
    ASSERT_EQ(map.size(), 2);
    ASSERT_EQ(map.at("width")->asInteger(), 1920);
    ASSERT_EQ(map.at("height")->asInteger(), 1080);
}

TEST_F(ParserTest, DynamicValue) {
    std::string source = R"(
[Config]
dyna_value = Dyna(100)
    )";
    
    auto parser = createParser(source);
    const auto& config = parser->getSections().at("Config");
    ASSERT_TRUE(config.entries.at("dyna_value")->isDynamic());
}

TEST_F(ParserTest, SchemaParsingAndValidation) {
    // This test ensures a schema is parsed AND that valid data passes validation.
    std::string source = R"(
[#schema]
[Visual]
width = !, int
height = ?, int

[Visual]
width = 1920

[Graphics]
width = 2560
    )";
    
    // This should parse and validate successfully.
    auto parser = createParser(source);

    // Check that the schema itself was parsed correctly.
    const auto& schema = parser->getSchema();
    ASSERT_NE(schema.find("Visual"), schema.end());
    ASSERT_EQ(schema.at("Visual").size(), 2);

    // Check that the data section was parsed correctly.
    const auto& sections = parser->getSections();
    ASSERT_NE(sections.find("Visual"), sections.end());
    ASSERT_EQ(sections.at("Visual").entries.at("width")->asInteger(), 1920);
}

TEST_F(ParserTest, CorrectlyParsesSpecialTypes) {
    // This test verifies that types specified in YINI.md are parsed with the correct, specific type tag.
    // This is the "failing test" for the type information loss bug.
    std::string source = R"(
[MyTypes]
p = Path("some/path")
l = List(1, 2)
s = (1, 2, 3)
t = (1, "a")
    )";

    auto parser = createParser(source);
    const auto& sections = parser->getSections();
    ASSERT_NE(sections.find("MyTypes"), sections.end());
    const auto& types = sections.at("MyTypes");

    // Verify Path type
    ASSERT_NE(types.entries.find("p"), types.entries.end());
    auto path_val = types.entries.at("p");
    EXPECT_TRUE(path_val->isPath()) << "Value should be a Path, but was " << path_val->toString();
    EXPECT_EQ(path_val->asString(), "some/path");

    // Verify List type
    ASSERT_NE(types.entries.find("l"), types.entries.end());
    auto list_val = types.entries.at("l");
    EXPECT_TRUE(list_val->isList()) << "Value should be a List, but was " << list_val->toString();
    ASSERT_EQ(list_val->asArray().size(), 2);

    // Verify Set type
    ASSERT_NE(types.entries.find("s"), types.entries.end());
    auto set_val = types.entries.at("s");
    EXPECT_TRUE(set_val->isSet()) << "Value should be a Set, but was " << set_val->toString();
    ASSERT_EQ(set_val->asArray().size(), 3);

    // Verify Tuple type
    ASSERT_NE(types.entries.find("t"), types.entries.end());
    auto tuple_val = types.entries.at("t");
    EXPECT_TRUE(tuple_val->isTuple()) << "Value should be a Tuple, but was " << tuple_val->toString();
    ASSERT_EQ(tuple_val->asArray().size(), 2);
}

// Test suite for parser error conditions
class ParserErrorTest : public ::testing::Test {
protected:
    void expect_error(const std::string& source, const std::string& expected_error_msg) {
        Parser parser(source);
        EXPECT_FALSE(parser.parse()) << "Parsing should have failed but succeeded.";
        ASSERT_TRUE(parser.hasError()) << "Parser did not report an error.";

        std::string actual_error = parser.getLastError();
        EXPECT_NE(actual_error.find(expected_error_msg), std::string::npos)
            << "Expected to find substring:\n\"" << expected_error_msg << "\"\n"
            << "In actual error message:\n\"" << actual_error << "\"";
    }
};

TEST_F(ParserErrorTest, MissingSectionClosingBracket) {
    expect_error("[Section", "Expected ']' after section name");
}

TEST_F(ParserErrorTest, MissingEqualsInPair) {
    expect_error("[S]\nk v", "Expected '=' after key");
}

TEST_F(ParserErrorTest, UnterminatedArray) {
    expect_error("[Test]\nk = [1, 2", "Expected ']' at end of array");
}

TEST_F(ParserErrorTest, UnresolvedMacroReference) {
    expect_error("[Test]\nk = @undefined", "Unresolved reference: undefined");
}

TEST_F(ParserErrorTest, UnresolvedSectionReference) {
    expect_error("[Test]\nk = @{Bad.ref}", "Reference to unknown section: Bad");
}

TEST_F(ParserErrorTest, CircularReference) {
    expect_error(R"(
[A]
a = @{B.b}
[B]
b = @{A.a}
    )", "Circular reference detected");
}

TEST_F(ParserErrorTest, SchemaRequiredKeyMissing) {
    // The data section [A] is now separate from the [#schema] block.
    // The parser will parse the schema, then parse the data, then fail validation.
    expect_error(R"(
[#schema]
[A]
key = !

[A]
other_key = 1
    )", "Required key 'key' not found in section [A]");
}

TEST_F(ParserErrorTest, SchemaWrongType) {
    // The data section [A] is now separate from the [#schema] block.
    expect_error(R"(
[#schema]
[A]
key = !, int

[A]
key = "a string"
    )", "has wrong type");
}


TEST_F(ParserTest, ComprehensiveReferenceResolution) {
    std::string source = R"(
[#define]
BASE_WIDTH = 1920

[Graphics]
width = @BASE_WIDTH
height = 1080
half_width = 960

[UI]
panel_width = @{Graphics.half_width}
screen_width = @{Graphics.width}

[Advanced]
resolution = [@{Graphics.width}, @{Graphics.height}]
    )";
    
    auto parser = createParser(source);
    const auto& sections = parser->getSections();
    
    const auto& graphics = sections.at("Graphics");
    ASSERT_EQ(graphics.entries.at("width")->asInteger(), 1920);
    
    const auto& ui = sections.at("UI");
    ASSERT_EQ(ui.entries.at("panel_width")->asInteger(), 960);
    ASSERT_EQ(ui.entries.at("screen_width")->asInteger(), 1920);
    
    const auto& advanced = sections.at("Advanced");
    ASSERT_TRUE(advanced.entries.at("resolution")->isArray());
    auto res_arr = advanced.entries.at("resolution")->asArray();
    ASSERT_EQ(res_arr.size(), 2);
    ASSERT_EQ(res_arr[0]->asInteger(), 1920);
    ASSERT_EQ(res_arr[1]->asInteger(), 1080);
}