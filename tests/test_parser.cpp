#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "Lexer/Lexer.h"
#include "Parser/Ast.h"
#include "Parser/Parser.h"

using namespace YINI;

TEST(ParserTest, BasicParsing) {
    std::string source = R"(
        [Section1]
        key1 = value1
        key2 = 123

        [Section2]
        key3 = "a string"
    )";

    Lexer lexer(source);
    // Provide a dummy file path so the parser can resolve relative includes.
    // The test executable runs from the `build` directory, so the base path
    // for our includes needs to point back to the `tests` directory.
    Parser parser(lexer, "../tests/dummy.yini");
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 2);

    // Check Section1
    const auto& section1 = ast->sections[0];
    EXPECT_EQ(section1.name, "Section1");
    ASSERT_EQ(section1.key_values.size(), 2);
    EXPECT_EQ(section1.key_values[0].key, "key1");
    EXPECT_EQ(std::get<std::string>(section1.key_values[0].value->value), "value1");
    EXPECT_EQ(section1.key_values[1].key, "key2");
    EXPECT_EQ(std::get<int64_t>(section1.key_values[1].value->value), 123);

    // Check Section2
    const auto& section2 = ast->sections[1];
    EXPECT_EQ(section2.name, "Section2");
    ASSERT_EQ(section2.key_values.size(), 1);
    EXPECT_EQ(section2.key_values[0].key, "key3");
    EXPECT_EQ(std::get<std::string>(section2.key_values[0].value->value), "a string");
}

TEST(ParserTest, SectionInheritance) {
    std::string source = R"(
        [Parent1]
        key1 = value1

        [Parent2]
        key2 = value2

        [Child : Parent1, Parent2]
        key3 = value3
    )";

    Lexer lexer(source);
    Parser parser(lexer, "dummy.yini");
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 3);

    const auto& child_section = ast->sections[2];
    EXPECT_EQ(child_section.name, "Child");

    ASSERT_EQ(child_section.parents.size(), 2);
    EXPECT_EQ(child_section.parents[0], "Parent1");
    EXPECT_EQ(child_section.parents[1], "Parent2");

    ASSERT_EQ(child_section.key_values.size(), 1);
    EXPECT_EQ(child_section.key_values[0].key, "key3");
    EXPECT_EQ(std::get<std::string>(child_section.key_values[0].value->value), "value3");
}

TEST(ParserTest, PlusEqualsSyntax) {
    std::string source = R"(
        [MyList]
        += "item1"
        += "item2"
    )";

    Lexer lexer(source);
    // Provide a dummy file path so the parser can resolve relative includes.
    // The test executable runs from the `build` directory.
    Parser parser(lexer, "build/dummy.yini");
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1);

    const auto& section = ast->sections[0];
    EXPECT_EQ(section.name, "MyList");
    ASSERT_EQ(section.key_values.size(), 2);

    EXPECT_EQ(section.key_values[0].key, "0");
    EXPECT_EQ(std::get<std::string>(section.key_values[0].value->value), "item1");

    EXPECT_EQ(section.key_values[1].key, "1");
    EXPECT_EQ(std::get<std::string>(section.key_values[1].value->value), "item2");
}

TEST(ParserTest, DataTypeParsing) {
    std::string source = R"(
        [Data]
        my_string = "hello"
        my_int = 42
        my_float = 3.14
        my_bool_true = true
        my_bool_false = false
        my_identifier = an_identifier
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 6);

    EXPECT_EQ(std::get<std::string>(section.key_values[0].value->value), "hello");
    EXPECT_EQ(std::get<int64_t>(section.key_values[1].value->value), 42);
    EXPECT_DOUBLE_EQ(std::get<double>(section.key_values[2].value->value), 3.14);
    EXPECT_EQ(std::get<bool>(section.key_values[3].value->value), true);
    EXPECT_EQ(std::get<bool>(section.key_values[4].value->value), false);
    EXPECT_EQ(std::get<std::string>(section.key_values[5].value->value), "an_identifier");
}

TEST(ParserTest, ArrayParsing) {
    std::string source = R"(
        [Data]
        my_array = [1, "two", 3.0, true]
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 1);

    const auto& kv = section.key_values[0];
    EXPECT_EQ(kv.key, "my_array");

    // We'll need to update YiniValue to support arrays first.
    // This test will fail until then.
    auto* array_ptr = std::get_if<std::unique_ptr<YiniValue::Array>>(&kv.value->value);
    ASSERT_TRUE(array_ptr);
    const auto& array = **array_ptr;
    ASSERT_EQ(array.size(), 4);

    EXPECT_EQ(std::get<int64_t>(array[0].value), 1);
    EXPECT_EQ(std::get<std::string>(array[1].value), "two");
    EXPECT_DOUBLE_EQ(std::get<double>(array[2].value), 3.0);
    EXPECT_EQ(std::get<bool>(array[3].value), true);
}

TEST(ParserTest, MapParsing) {
    std::string source = R"(
        [Data]
        my_map = {
            "name": "Jules",
            "level": 99,
            "items": ["sword", "shield"]
        }
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 1);

    const auto& kv = section.key_values[0];
    EXPECT_EQ(kv.key, "my_map");

    // This test will fail until we add map support to YiniValue and the parser.
    auto* map_ptr = std::get_if<std::unique_ptr<YiniValue::Map>>(&kv.value->value);
    ASSERT_TRUE(map_ptr);
    const auto& map = **map_ptr;

    ASSERT_EQ(map.size(), 3);

    EXPECT_EQ(std::get<std::string>(map.at("name").value), "Jules");
    EXPECT_EQ(std::get<int64_t>(map.at("level").value), 99);

    const auto& items_value = map.at("items");
    auto* items_array_ptr = std::get_if<std::unique_ptr<YiniValue::Array>>(&items_value.value);
    ASSERT_TRUE(items_array_ptr);
    const auto& items_array = **items_array_ptr;
    ASSERT_EQ(items_array.size(), 2);
    EXPECT_EQ(std::get<std::string>(items_array[0].value), "sword");
    EXPECT_EQ(std::get<std::string>(items_array[1].value), "shield");
}

TEST(ParserTest, MacroParsing) {
    std::string source = R"(
        [#define]
        primary_color = "blue"
        width = 1024

        [Window]
        color = @primary_color
        w = @width
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1); // Only [Window] is a regular section.

    const auto& section = ast->sections[0];
    EXPECT_EQ(section.name, "Window");
    ASSERT_EQ(section.key_values.size(), 2);

    const auto& color_kv = section.key_values[0];
    EXPECT_EQ(color_kv.key, "color");
    EXPECT_EQ(std::get<std::string>(color_kv.value->value), "blue");

    const auto& width_kv = section.key_values[1];
    EXPECT_EQ(width_kv.key, "w");
    EXPECT_EQ(std::get<int64_t>(width_kv.value->value), 1024);
}

TEST(ParserTest, DynaValueParsing) {
    std::string source = R"(
        [Settings]
        volume = Dyna(1.0)
        fullscreen = dyna(false)
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Check volume
    const auto& vol_kv = section.key_values[0];
    auto* vol_wrapper = std::get_if<YiniValue::DynaWrapper>(&vol_kv.value->value);
    ASSERT_TRUE(vol_wrapper);
    auto* vol_ptr = std::get_if<double>(&(*vol_wrapper)->value);
    ASSERT_TRUE(vol_ptr);
    EXPECT_DOUBLE_EQ(*vol_ptr, 1.0);

    // Check fullscreen
    const auto& fs_kv = section.key_values[1];
    auto* fs_wrapper = std::get_if<YiniValue::DynaWrapper>(&fs_kv.value->value);
    ASSERT_TRUE(fs_wrapper);
    auto* fs_ptr = std::get_if<bool>(&(*fs_wrapper)->value);
    ASSERT_TRUE(fs_ptr);
    EXPECT_EQ(*fs_ptr, false);
}

TEST(ParserTest, IncludeParsing) {
    // Note: This test assumes the test executable is run from the `build` directory.
    std::string source = R"(
        [#include]
        += "include_base.yini"
        += "include_override.yini"

        [Main]
        setting = "main_value"
    )";

    Lexer lexer(source);
    // Provide an absolute dummy file path to the parser so it can resolve relative includes.
    Parser parser(lexer, "/app/tests/dummy.yini");
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);

    // Check macros - version should be overridden
    ASSERT_EQ(ast->macros.size(), 1);
    EXPECT_EQ(std::get<std::string>(ast->macros.at("version")->value), "1.1");

    // Check sections - should have Settings from included files and Main from the main file
    ASSERT_EQ(ast->sections.size(), 2);

    const auto& settings_section = ast->sections[0];
    EXPECT_EQ(settings_section.name, "Settings");
    ASSERT_EQ(settings_section.key_values.size(), 3);

    // Check that values are correctly overridden
    bool found_fullscreen = false;
    bool found_sound = false;
    bool found_user = false;

    for (const auto& kv : settings_section.key_values) {
        if (kv.key == "fullscreen") {
            EXPECT_EQ(std::get<bool>(kv.value->value), true); // overridden
            found_fullscreen = true;
        } else if (kv.key == "sound") {
            EXPECT_EQ(std::get<int64_t>(kv.value->value), 100); // from base
            found_sound = true;
        } else if (kv.key == "user") {
            EXPECT_EQ(std::get<std::string>(kv.value->value), "override"); // overridden
            found_user = true;
        }
    }
    EXPECT_TRUE(found_fullscreen);
    EXPECT_TRUE(found_sound);
    EXPECT_TRUE(found_user);

    // Check Main section
    const auto& main_section = ast->sections[1];
    EXPECT_EQ(main_section.name, "Main");
    ASSERT_EQ(main_section.key_values.size(), 1);
    EXPECT_EQ(main_section.key_values[0].key, "setting");
    EXPECT_EQ(std::get<std::string>(main_section.key_values[0].value->value), "main_value");
}

TEST(ParserTest, ArithmeticParsing) {
    std::string source = R"(
        [#define]
        width = 1024
        height = 768

        [Data]
        val1 = 10 + 5 * 2 // 20
        val2 = (10 + 5) * 2 // 30
        val3 = @width / 2 // 512
        val4 = 10.5 - 0.5 // 10.0
        val5 = 10 % 3 // 1
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->sections.size(), 1);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 5);

    EXPECT_EQ(std::get<int64_t>(section.key_values[0].value->value), 20);
    EXPECT_EQ(std::get<int64_t>(section.key_values[1].value->value), 30);
    EXPECT_EQ(std::get<int64_t>(section.key_values[2].value->value), 512);
    EXPECT_DOUBLE_EQ(std::get<double>(section.key_values[3].value->value), 10.0);
    EXPECT_EQ(std::get<int64_t>(section.key_values[4].value->value), 1);
}

TEST(ParserTest, FunctionStyleCasting) {
    std::string source = R"(
        [Data]
        my_array = Array(1, 2, 3)
        my_list = list("a", "b", "c")
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Check Array
    const auto& array_kv = section.key_values[0];
    auto* array_ptr = std::get_if<std::unique_ptr<YiniValue::Array>>(&array_kv.value->value);
    ASSERT_TRUE(array_ptr);
    const auto& array = **array_ptr;
    ASSERT_EQ(array.size(), 3);
    EXPECT_EQ(std::get<int64_t>(array[0].value), 1);

    // Check List
    const auto& list_kv = section.key_values[1];
    auto* list_ptr = std::get_if<std::unique_ptr<YiniValue::Array>>(&list_kv.value->value);
    ASSERT_TRUE(list_ptr);
    const auto& list = **list_ptr;
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(std::get<std::string>(list[0].value), "a");
}

TEST(ParserTest, SetParsing) {
    std::string source = R"(
        [Data]
        my_set = (1, "two", 1) // Note: sets can have duplicates at parse time
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 1);

    // We'll treat sets as arrays for now at the AST level.
    // The distinction can be made during semantic analysis or at runtime.
    const auto& set_kv = section.key_values[0];
    auto* set_ptr = std::get_if<std::unique_ptr<YiniValue::Array>>(&set_kv.value->value);
    ASSERT_TRUE(set_ptr);
    const auto& set_val = **set_ptr;
    ASSERT_EQ(set_val.size(), 3);
    EXPECT_EQ(std::get<int64_t>(set_val[0].value), 1);
    EXPECT_EQ(std::get<std::string>(set_val[1].value), "two");
    EXPECT_EQ(std::get<int64_t>(set_val[2].value), 1);
}

TEST(ParserTest, ColorParsing) {
    std::string source = R"(
        [Data]
        color_hex = #FF00AA
        color_func = color(255, 128, 0)
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Check hex color
    const auto& hex_kv = section.key_values[0];
    auto* hex_ptr = std::get_if<Color>(&hex_kv.value->value);
    ASSERT_TRUE(hex_ptr);
    EXPECT_EQ(*hex_ptr, (Color{255, 0, 170}));

    // Check function-style color
    const auto& func_kv = section.key_values[1];
    auto* func_ptr = std::get_if<Color>(&func_kv.value->value);
    ASSERT_TRUE(func_ptr);
    EXPECT_EQ(*func_ptr, (Color{255, 128, 0}));
}

TEST(ParserTest, CoordParsing) {
    std::string source = R"(
        [Data]
        pos2d = Coord(1.0, 2.5)
        pos3d = coord(10, 20, 30)
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Check 2D Coord
    const auto& pos2d_kv = section.key_values[0];
    auto* pos2d_ptr = std::get_if<Coord>(&pos2d_kv.value->value);
    ASSERT_TRUE(pos2d_ptr);
    EXPECT_EQ(*pos2d_ptr, (Coord{1.0, 2.5, std::nullopt}));

    // Check 3D Coord
    const auto& pos3d_kv = section.key_values[1];
    auto* pos3d_ptr = std::get_if<Coord>(&pos3d_kv.value->value);
    ASSERT_TRUE(pos3d_ptr);
    EXPECT_EQ(*pos3d_ptr, (Coord{10.0, 20.0, 30.0}));
}

TEST(ParserTest, PathParsing) {
    std::string source = R"(
        [Data]
        asset_path = path("assets/player.png")
        another_path = Path("/usr/local/bin")
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Check first path
    const auto& path1_kv = section.key_values[0];
    auto* path1_ptr = std::get_if<Path>(&path1_kv.value->value);
    ASSERT_TRUE(path1_ptr);
    EXPECT_EQ(path1_ptr->value, "assets/player.png");

    // Check second path
    const auto& path2_kv = section.key_values[1];
    auto* path2_ptr = std::get_if<Path>(&path2_kv.value->value);
    ASSERT_TRUE(path2_ptr);
    EXPECT_EQ(path2_ptr->value, "/usr/local/bin");
}

TEST(ParserTest, CrossSectionReferenceParsing) {
    std::string source = R"(
        [Config]
        width = 1920
        height = 1080

        [Window]
        w = @{Config.width}
        h = @{Config.height}
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[1]; // Window section
    ASSERT_EQ(section.key_values.size(), 2);

    // Check first reference
    const auto& ref1_kv = section.key_values[0];
    EXPECT_EQ(ref1_kv.key, "w");
    auto* ref1_ptr = std::get_if<CrossSectionRef>(&ref1_kv.value->value);
    ASSERT_TRUE(ref1_ptr);
    EXPECT_EQ(*ref1_ptr, (CrossSectionRef{"Config", "width"}));

    // Check second reference
    const auto& ref2_kv = section.key_values[1];
    EXPECT_EQ(ref2_kv.key, "h");
    auto* ref2_ptr = std::get_if<CrossSectionRef>(&ref2_kv.value->value);
    ASSERT_TRUE(ref2_ptr);
    EXPECT_EQ(*ref2_ptr, (CrossSectionRef{"Config", "height"}));
}

TEST(ParserTest, EnvVarParsing) {
    // Set environment variables for the test
    setenv("TEST_WIDTH", "1920", 1);
    setenv("TEST_GREETING", "hello", 1);

    std::string source = R"(
        [Window]
        width = ${TEST_WIDTH}
        greeting = ${TEST_GREETING}
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    std::unique_ptr<AstNode> ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[0];
    ASSERT_EQ(section.key_values.size(), 2);

    // Note: Env vars are substituted as strings. Further parsing might be needed.
    // For now, we'll check for the raw string value.
    const auto& width_kv = section.key_values[0];
    EXPECT_EQ(std::get<std::string>(width_kv.value->value), "1920");

    const auto& greeting_kv = section.key_values[1];
    EXPECT_EQ(std::get<std::string>(greeting_kv.value->value), "hello");

    // Clean up environment variables
    unsetenv("TEST_WIDTH");
    unsetenv("TEST_GREETING");
}