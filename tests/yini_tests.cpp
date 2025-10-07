#include <gtest/gtest.h>
#include "Parser.h"
#include "Interpreter.h"
#include "Value.h"

using namespace yini;

// Helper function to run parser and interpreter
bool parseAndInterpret(const std::string& source, Interpreter& interpreter) {
    Parser parser(source);
    auto ast = parser.parse();
    if (parser.hasError()) {
        // Use GTest's logging for better visibility
        ADD_FAILURE() << "Parser error: " << parser.getLastError();
        return false;
    }
    EXPECT_NE(ast, nullptr);
    if (!ast) return false;

    bool success = interpreter.interpret(*ast);
    if (!success) {
        ADD_FAILURE() << "Interpreter error: " << interpreter.getLastError();
    }
    return success;
}

TEST(InterpreterTest, SimpleSection) {
    std::string source = R"(
[Config]
key1 = 123
key2 = "value"
key3 = true
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
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

TEST(InterpreterTest, Arrays) {
    std::string source = R"(
[Config]
arr = [1, 2, 3]
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& config = sections.at("Config");

    ASSERT_TRUE(config.entries.at("arr")->isArray());
    auto arr = config.entries.at("arr")->asArray();
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0]->asInteger(), 1);
    EXPECT_EQ(arr[1]->asInteger(), 2);
    EXPECT_EQ(arr[2]->asInteger(), 3);
}

TEST(InterpreterTest, Inheritance) {
    std::string source = R"(
[Base]
key1 = 100
key2 = 200

[Derived] : Base
key2 = 300
key3 = 400
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& derived = sections.at("Derived");

    ASSERT_NE(derived.entries.find("key1"), derived.entries.end());
    EXPECT_EQ(derived.entries.at("key1")->asInteger(), 100);

    EXPECT_EQ(derived.entries.at("key2")->asInteger(), 300);

    EXPECT_EQ(derived.entries.at("key3")->asInteger(), 400);
}

TEST(InterpreterTest, QuickRegister) {
    std::string source = R"(
[Registry]
+= "value1"
+= "value2"
+= "value3"
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& registry = sections.at("Registry");

    ASSERT_EQ(registry.entries.size(), 3);
    EXPECT_EQ(registry.entries.at("0")->asString(), "value1");
    EXPECT_EQ(registry.entries.at("1")->asString(), "value2");
    EXPECT_EQ(registry.entries.at("2")->asString(), "value3");
}

TEST(InterpreterTest, Arithmetic) {
    std::string source = R"(
[Math]
add = 1 + 2
multiply = 3 * 4
complex = 1 + 2 * 3
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& math = sections.at("Math");

    EXPECT_EQ(math.entries.at("add")->asInteger(), 3);
    EXPECT_EQ(math.entries.at("multiply")->asInteger(), 12);
    EXPECT_EQ(math.entries.at("complex")->asInteger(), 7);
}

TEST(InterpreterTest, Color) {
    std::string source = R"(
[Visual]
color1 = #FF0000
color2 = Color(255, 0, 0)
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& visual = sections.at("Visual");

    ASSERT_TRUE(visual.entries.at("color1")->isColor());
    auto c1 = visual.entries.at("color1")->asColor();
    EXPECT_EQ(c1.r, 255);
    EXPECT_EQ(c1.g, 0);
    EXPECT_EQ(c1.b, 0);

    ASSERT_TRUE(visual.entries.at("color2")->isColor());
    auto c2 = visual.entries.at("color2")->asColor();
    EXPECT_EQ(c2.r, 255);
    EXPECT_EQ(c2.g, 0);
    EXPECT_EQ(c2.b, 0);
}

TEST(InterpreterTest, Coord) {
    std::string source = R"(
[Position]
pos2d = Coord(10, 20)
pos3d = Coord(10, 20, 30)
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& position = sections.at("Position");

    ASSERT_TRUE(position.entries.at("pos2d")->isCoord());
    auto c2d = position.entries.at("pos2d")->asCoord();
    EXPECT_EQ(c2d.x, 10);
    EXPECT_EQ(c2d.y, 20);
    EXPECT_FALSE(c2d.z.has_value());

    ASSERT_TRUE(position.entries.at("pos3d")->isCoord());
    auto c3d = position.entries.at("pos3d")->asCoord();
    EXPECT_EQ(c3d.x, 10);
    EXPECT_EQ(c3d.y, 20);
    ASSERT_TRUE(c3d.z.has_value());
    EXPECT_EQ(c3d.z.value(), 30);
}

TEST(InterpreterTest, Defines) {
    std::string source = R"(
[#define]
width = 1920
height = 1080

[Config]
key1 = @width
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& defines = interpreter.getDefines();
    ASSERT_NE(defines.find("width"), defines.end());
    EXPECT_EQ(defines.at("width")->asInteger(), 1920);
    EXPECT_EQ(defines.at("height")->asInteger(), 1080);

    const auto& sections = interpreter.getSections();
    const auto& config = sections.at("Config");
    ASSERT_NE(config.entries.find("key1"), config.entries.end());

    auto key1_value = config.entries.at("key1");
    ASSERT_TRUE(key1_value->isInteger());
    EXPECT_EQ(key1_value->asInteger(), 1920);
}

TEST(InterpreterTest, Includes) {
    std::string source = R"(
[#include]
+= "file1.yini"
+= "file2.yini"
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& includes = interpreter.getIncludes();
    ASSERT_EQ(includes.size(), 2);
    EXPECT_EQ(includes[0], "file1.yini");
    EXPECT_EQ(includes[1], "file2.yini");
}

TEST(InterpreterTest, Map) {
    std::string source = R"(
[Config]
settings = {width: 1920, height: 1080}
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& config = sections.at("Config");

    ASSERT_TRUE(config.entries.at("settings")->isMap());
    auto map = config.entries.at("settings")->asMap();
    ASSERT_EQ(map.size(), 2);
    EXPECT_EQ(map.at("width")->asInteger(), 1920);
    EXPECT_EQ(map.at("height")->asInteger(), 1080);
}

TEST(InterpreterTest, DynamicValues) {
    std::string source = R"(
[Config]
dyna_value = Dyna(100)
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& config = sections.at("Config");

    ASSERT_TRUE(config.entries.at("dyna_value")->isDynamic());
}

TEST(InterpreterTest, SchemaValidation) {
    std::string source = R"(
[Graphics]
width = 2560
height = 1440
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& graphics = sections.at("Graphics");

    ASSERT_NE(graphics.entries.find("width"), graphics.entries.end());
    EXPECT_EQ(graphics.entries.at("width")->asInteger(), 2560);

    ASSERT_NE(graphics.entries.find("height"), graphics.entries.end());
    EXPECT_EQ(graphics.entries.at("height")->asInteger(), 1440);
}

TEST(InterpreterTest, CrossSectionReference) {
    std::string source = R"(
[Config]
width = 1920
height = 1080

[Display]
screen_width = @{Config.width}
screen_height = @{Config.height}
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();
    const auto& display = sections.at("Display");

    ASSERT_NE(display.entries.find("screen_width"), display.entries.end());
    ASSERT_TRUE(display.entries.at("screen_width")->isInteger());
    EXPECT_EQ(display.entries.at("screen_width")->asInteger(), 1920);

    ASSERT_NE(display.entries.find("screen_height"), display.entries.end());
    ASSERT_TRUE(display.entries.at("screen_height")->isInteger());
    EXPECT_EQ(display.entries.at("screen_height")->asInteger(), 1080);
}

TEST(InterpreterTest, ComprehensiveReferenceResolution) {
    std::string source = R"(
[#define]
BASE_WIDTH = 1920
BASE_HEIGHT = 1080

[Graphics]
width = @BASE_WIDTH
height = @BASE_HEIGHT
half_width = 960
aspect_ratio = 1.777

[UI]
panel_width = @{Graphics.half_width}
screen_width = @{Graphics.width}
screen_height = @{Graphics.height}

[Advanced]
resolution = [@{Graphics.width}, @{Graphics.height}]
    )";

    Interpreter interpreter;
    ASSERT_TRUE(parseAndInterpret(source, interpreter));

    const auto& sections = interpreter.getSections();

    const auto& graphics = sections.at("Graphics");
    EXPECT_EQ(graphics.entries.at("width")->asInteger(), 1920);
    EXPECT_EQ(graphics.entries.at("height")->asInteger(), 1080);

    const auto& ui = sections.at("UI");
    EXPECT_EQ(ui.entries.at("panel_width")->asInteger(), 960);
    EXPECT_EQ(ui.entries.at("screen_width")->asInteger(), 1920);
    EXPECT_EQ(ui.entries.at("screen_height")->asInteger(), 1080);

    const auto& advanced = sections.at("Advanced");
    ASSERT_TRUE(advanced.entries.at("resolution")->isArray());
    auto res_arr = advanced.entries.at("resolution")->asArray();
    ASSERT_EQ(res_arr.size(), 2);
    EXPECT_EQ(res_arr[0]->asInteger(), 1920);
    EXPECT_EQ(res_arr[1]->asInteger(), 1080);
}

// New tests for error conditions

TEST(ErrorHandlingTest, ParserUnterminatedString) {
    std::string source = R"([Config] key = "unterminated)";
    Parser parser(source);
    parser.parse();
    ASSERT_TRUE(parser.hasError());
    EXPECT_NE(parser.getLastError().find("Unterminated string"), std::string::npos);
}

TEST(ErrorHandlingTest, ParserInvalidToken) {
    std::string source = R"([Config] key = ^)";
    Parser parser(source);
    parser.parse();
    ASSERT_TRUE(parser.hasError());
    EXPECT_NE(parser.getLastError().find("Unexpected character"), std::string::npos);
}

TEST(ErrorHandlingTest, InterpreterCircularReference) {
    std::string source = R"(
[A]
ref = @{B.ref}
[B]
ref = @{A.ref}
    )";
    Parser parser(source);
    auto ast = parser.parse();
    ASSERT_FALSE(parser.hasError()) << "Parser error: " << parser.getLastError();
    ASSERT_NE(ast, nullptr);

    Interpreter interpreter;
    bool success = interpreter.interpret(*ast);
    ASSERT_FALSE(success);
    EXPECT_NE(interpreter.getLastError().find("Circular reference"), std::string::npos);
}