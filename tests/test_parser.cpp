#include <gtest/gtest.h>
#include "Parser/parser.h"
#include <stdlib.h>

TEST(ParserTest, ParseSectionAndKeyValuePair) {
    std::string input = "[Config]\nkey = \"value\"";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Config"));
    EXPECT_EQ(std::get<std::string>(config["Config"]["key"]), "value");
}

TEST(ParserTest, ParseInteger) {
    std::string input = "[Numbers]\nint_val = 123";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_EQ(std::get<int>(config["Numbers"]["int_val"]), 123);
}

TEST(ParserTest, ParseFloat) {
    std::string input = "[Numbers]\nfloat_val = 3.14";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_DOUBLE_EQ(std::get<double>(config["Numbers"]["float_val"]), 3.14);
}

TEST(ParserTest, ParseBoolean) {
    std::string input = "[Booleans]\nbool_true = true\nbool_false = false";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Booleans"));
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_true"]), true);
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_false"]), false);
}

TEST(ParserTest, ParseArray) {
    std::string input = "[Data]\narray_val = [1, \"two\", 3.14, true]";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Data"));

    auto& arr_variant = config["Data"]["array_val"];
    auto* arr_ptr = std::get<std::unique_ptr<Array>>(arr_variant).get();

    ASSERT_EQ(arr_ptr->elements.size(), 4);
    EXPECT_EQ(std::get<int>(arr_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<std::string>(arr_ptr->elements[1]), "two");
    EXPECT_DOUBLE_EQ(std::get<double>(arr_ptr->elements[2]), 3.14);
    EXPECT_EQ(std::get<bool>(arr_ptr->elements[3]), true);
}

TEST(ParserTest, ParseNestedArray) {
    std::string input = "[Data]\nnested_array = [[1, 2], [3, 4]]";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Data"));

    auto& nested_arr_variant = config["Data"]["nested_array"];
    auto* nested_arr_ptr = std::get<std::unique_ptr<Array>>(nested_arr_variant).get();

    ASSERT_EQ(nested_arr_ptr->elements.size(), 2);

    auto* inner_arr_1_ptr = std::get<std::unique_ptr<Array>>(nested_arr_ptr->elements[0]).get();
    ASSERT_EQ(inner_arr_1_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<int>(inner_arr_1_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<int>(inner_arr_1_ptr->elements[1]), 2);

    auto* inner_arr_2_ptr = std::get<std::unique_ptr<Array>>(nested_arr_ptr->elements[1]).get();
    ASSERT_EQ(inner_arr_2_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<int>(inner_arr_2_ptr->elements[0]), 3);
    EXPECT_EQ(std::get<int>(inner_arr_2_ptr->elements[1]), 4);
}

TEST(ParserTest, SectionInheritance) {
    std::string input = R"(
[Base1]
key1 = "value1"
key2 = 100

[Base2]
key2 = 200
key3 = true

[Derived] : Base1, Base2
key4 = "derived_value"
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Derived"));
    auto& derived_section = config["Derived"];

    EXPECT_EQ(std::get<std::string>(derived_section["key1"]), "value1");
    EXPECT_EQ(std::get<int>(derived_section["key2"]), 200);
    EXPECT_EQ(std::get<bool>(derived_section["key3"]), true);
    EXPECT_EQ(std::get<std::string>(derived_section["key4"]), "derived_value");
}

TEST(ParserTest, QuickRegistration) {
    std::string input = R"(
[Registry]
+= "item1"
+= 123
+= false
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Registry"));
    auto& registry_section = config["Registry"];

    ASSERT_TRUE(registry_section.count(""));
    auto& reg_variant = registry_section.at("");
    auto* reg_ptr = std::get<std::unique_ptr<Array>>(reg_variant).get();

    ASSERT_EQ(reg_ptr->elements.size(), 3);
    EXPECT_EQ(std::get<std::string>(reg_ptr->elements[0]), "item1");
    EXPECT_EQ(std::get<int>(reg_ptr->elements[1]), 123);
    EXPECT_EQ(std::get<bool>(reg_ptr->elements[2]), false);
}

TEST(ParserTest, ParseSet) {
    std::string input = "[Data]\nset_val = (1, \"two\", true)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Data"));

    auto& set_variant = config["Data"]["set_val"];
    auto* set_ptr = std::get<std::unique_ptr<Set>>(set_variant).get();

    ASSERT_EQ(set_ptr->elements.size(), 3);
    EXPECT_EQ(std::get<int>(set_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<std::string>(set_ptr->elements[1]), "two");
    EXPECT_EQ(std::get<bool>(set_ptr->elements[2]), true);
}

TEST(ParserTest, ParseMap) {
    std::string input = "[Data]\nmap_val = {key1: \"value1\", key2: 123, key3: true}";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Data"));

    auto& map_variant = config["Data"]["map_val"];
    auto* map_ptr = std::get<std::unique_ptr<Map>>(map_variant).get();

    ASSERT_EQ(map_ptr->elements.size(), 3);
    EXPECT_EQ(std::get<std::string>(map_ptr->elements["key1"]), "value1");
    EXPECT_EQ(std::get<int>(map_ptr->elements["key2"]), 123);
    EXPECT_EQ(std::get<bool>(map_ptr->elements["key3"]), true);
}

TEST(ParserTest, ParseExplicitListAndArray) {
    std::string input = R"(
[Data]
list_val = List(1, "two")
array_val = Array(true, 3.14)
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Data"));

    auto& list_variant = config["Data"]["list_val"];
    auto* list_ptr = std::get<std::unique_ptr<Array>>(list_variant).get();
    ASSERT_EQ(list_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<int>(list_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<std::string>(list_ptr->elements[1]), "two");

    auto& array_variant = config["Data"]["array_val"];
    auto* array_ptr = std::get<std::unique_ptr<Array>>(array_variant).get();
    ASSERT_EQ(array_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<bool>(array_ptr->elements[0]), true);
    EXPECT_DOUBLE_EQ(std::get<double>(array_ptr->elements[1]), 3.14);
}

TEST(ParserTest, MacroDefinitionAndReference) {
    std::string input = R"(
[#define]
brand_color = "#FF5733"
default_width = 800

[UI]
background_color = @brand_color
width = @default_width
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("UI"));
    auto& ui_section = config["UI"];

    EXPECT_EQ(std::get<std::string>(ui_section["background_color"]), "#FF5733");
    EXPECT_EQ(std::get<int>(ui_section["width"]), 800);

    EXPECT_FALSE(config.count("#define"));
}

TEST(ParserTest, EnvironmentVariableReference) {
    // Set environment variables for the test
    setenv("TEST_WIDTH", "1920", 1);
    setenv("TEST_ENABLED", "true", 1);
    setenv("TEST_THEME", "\"dark\"", 1); // Test with quotes in the value

    std::string input = R"(
[Visual]
width = ${TEST_WIDTH}
enabled = ${TEST_ENABLED}
theme = ${TEST_THEME}
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Visual"));
    auto& visual_section = config["Visual"];

    EXPECT_EQ(std::get<int>(visual_section["width"]), 1920);
    EXPECT_EQ(std::get<bool>(visual_section["enabled"]), true);
    EXPECT_EQ(std::get<std::string>(visual_section["theme"]), "dark");
}

TEST(ParserTest, CrossSectionReference) {
    std::string input = R"(
[Source]
width = 1920
height = 1080
enabled = true

[Target]
source_width = @{Source.width}
source_height = @{Source.height}
is_enabled = @{Source.enabled}
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Target"));
    auto& target_section = config["Target"];

    EXPECT_EQ(std::get<int>(target_section["source_width"]), 1920);
    EXPECT_EQ(std::get<int>(target_section["source_height"]), 1080);
    EXPECT_EQ(std::get<bool>(target_section["is_enabled"]), true);
}


TEST(ParserTest, FileInclusion) {
    std::string filepath = "tests/main_include.yini";
    Parser parser;

    auto config = parser.parseFile(filepath);

    ASSERT_TRUE(config.count("Settings"));
    EXPECT_EQ(std::get<double>(config["Settings"]["volume"]), 0.8);

    EXPECT_EQ(std::get<bool>(config["Settings"]["fullscreen"]), true);
    EXPECT_EQ(std::get<std::string>(config["Settings"]["theme"]), "light");

    ASSERT_TRUE(config.count("User"));
    EXPECT_EQ(std::get<std::string>(config["User"]["name"]), "Jules");

    ASSERT_TRUE(config.count("Graphics"));
    EXPECT_EQ(std::get<std::string>(config["Graphics"]["resolution"]), "1920x1080");
}

TEST(ParserTest, SchemaParsing) {
    std::string filepath = "tests/schema.yini";
    Parser parser;
    parser.parseFile(filepath);

    const auto& schema = parser.getSchema();

    ASSERT_TRUE(schema.count("Visual"));
    const auto& visual_schema = schema.at("Visual");

    // Test 'width' rule
    ASSERT_TRUE(visual_schema.count("width"));
    const auto& width_rule = visual_schema.at("width");
    EXPECT_TRUE(width_rule.required);
    EXPECT_EQ(width_rule.type.value(), "int");
    EXPECT_EQ(std::get<int>(width_rule.default_value.value()), 1280);
    EXPECT_EQ(width_rule.min_val.value(), 800);
    EXPECT_EQ(width_rule.max_val.value(), 1920);

    // Test 'height' rule
    ASSERT_TRUE(visual_schema.count("height"));
    const auto& height_rule = visual_schema.at("height");
    EXPECT_FALSE(height_rule.required);
    EXPECT_EQ(height_rule.type.value(), "int");
    EXPECT_FALSE(height_rule.default_value.has_value());

    // Test 'isOld' rule
    ASSERT_TRUE(visual_schema.count("isOld"));
    const auto& isOld_rule = visual_schema.at("isOld");
    EXPECT_TRUE(isOld_rule.required);
    EXPECT_EQ(isOld_rule.type.value(), "bool");
    EXPECT_EQ(isOld_rule.empty_behavior, 'e');

    // Test 'render_mode' rule
    ASSERT_TRUE(visual_schema.count("render_mode"));
    const auto& render_mode_rule = visual_schema.at("render_mode");
    EXPECT_FALSE(render_mode_rule.required);
    EXPECT_EQ(render_mode_rule.type.value(), "string");
    EXPECT_EQ(render_mode_rule.empty_behavior, '~');

    ASSERT_TRUE(schema.count("Audio"));
}

TEST(ParserTest, ArithmeticOperations) {
    std::string input = R"(
[#define]
base_width = 100

[Calculations]
simple_add = 5 + 3
simple_mul = 10 * 2
precedence = 5 + 3 * 2
parentheses = (5 + 3) * 2
with_float = 10 / 4.0
with_macro = @base_width * 2 + 50
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Calculations"));
    auto& calc_section = config["Calculations"];

    EXPECT_EQ(std::get<int>(calc_section["simple_add"]), 8);
    EXPECT_EQ(std::get<int>(calc_section["simple_mul"]), 20);
    EXPECT_EQ(std::get<int>(calc_section["precedence"]), 11); // 5 + 6
    EXPECT_EQ(std::get<int>(calc_section["parentheses"]), 16); // 8 * 2
    EXPECT_DOUBLE_EQ(std::get<double>(calc_section["with_float"]), 2.5);
    EXPECT_EQ(std::get<int>(calc_section["with_macro"]), 250); // 100 * 2 + 50
}

TEST(ParserTest, SpecialValueTypes) {
    std::string input = R"(
[Assets]
player_color = color(255, 100, 50)
enemy_color = #FF6432 // Same color as hex
spawn_point = Coord(10, -20, 5)
texture_path = path("textures/player.png")
)";
    Parser parser;
    auto config = parser.parse(input);

    ASSERT_TRUE(config.count("Assets"));
    auto& assets_section = config["Assets"];

    // Test Color
    auto& player_color_variant = assets_section["player_color"];
    auto player_color = std::get<Color>(player_color_variant);
    EXPECT_EQ(player_color.r, 255);
    EXPECT_EQ(player_color.g, 100);
    EXPECT_EQ(player_color.b, 50);

    auto& enemy_color_variant = assets_section["enemy_color"];
    auto enemy_color = std::get<Color>(enemy_color_variant);
    EXPECT_EQ(enemy_color.r, 255);
    EXPECT_EQ(enemy_color.g, 100);
    EXPECT_EQ(enemy_color.b, 50);

    // Test Coord
    auto& spawn_point_variant = assets_section["spawn_point"];
    auto spawn_point = std::get<Coord>(spawn_point_variant);
    EXPECT_EQ(spawn_point.x, 10);
    EXPECT_EQ(spawn_point.y, -20);
    EXPECT_EQ(spawn_point.z, 5);

    // Test Path
    auto& texture_path_variant = assets_section["texture_path"];
    auto texture_path = std::get<Path>(texture_path_variant);
    EXPECT_EQ(texture_path.value, "textures/player.png");
}