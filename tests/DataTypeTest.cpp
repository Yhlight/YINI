#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <variant>

// Helper to create a file and load it with a YiniManager
static void load_manager_from_source(YINI::YiniManager& manager, const std::string& filename, const std::string& source) {
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();
    manager.load(filename);
}

TEST(DataTypeTest, HandlesColorTypeCorrectly)
{
    std::string source = R"(
        [MySection]
        my_color = Color(255, 128, 64, 255)
    )";

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_color.yini", source);
    const auto& section = manager.get_interpreter().resolved_sections.find("MySection")->second;
    const auto& color_val = section.find("my_color")->second;
    auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&color_val.m_value);
    ASSERT_NE(map_ptr, nullptr);
    const auto& map = **map_ptr;

    EXPECT_EQ(std::get<double>(map.find("r")->second.m_value), 255);
    EXPECT_EQ(std::get<double>(map.find("g")->second.m_value), 128);
    EXPECT_EQ(std::get<double>(map.find("b")->second.m_value), 64);
    EXPECT_EQ(std::get<double>(map.find("a")->second.m_value), 255);
}

TEST(DataTypeTest, HandlesVectorTypesCorrectly)
{
    std::string source = R"(
        [MySection]
        my_vec2 = Vec2(1.0, 2.5)
        my_vec3 = Vec3(1, 2, 3)
        my_vec4 = Vec4(10, 20, 30, 40)
    )";

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_vectors.yini", source);
    const auto& section = manager.get_interpreter().resolved_sections.find("MySection")->second;

    // Test Vec2
    const auto& vec2_val = section.find("my_vec2")->second;
    auto* v2_map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&vec2_val.m_value);
    ASSERT_NE(v2_map_ptr, nullptr);
    const auto& v2_map = **v2_map_ptr;
    EXPECT_EQ(std::get<double>(v2_map.find("x")->second.m_value), 1.0);
    EXPECT_EQ(std::get<double>(v2_map.find("y")->second.m_value), 2.5);

    // Test Vec3
    const auto& vec3_val = section.find("my_vec3")->second;
    auto* v3_map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&vec3_val.m_value);
    ASSERT_NE(v3_map_ptr, nullptr);
    const auto& v3_map = **v3_map_ptr;
    EXPECT_EQ(std::get<double>(v3_map.find("x")->second.m_value), 1);
    EXPECT_EQ(std::get<double>(v3_map.find("y")->second.m_value), 2);
    EXPECT_EQ(std::get<double>(v3_map.find("z")->second.m_value), 3);

    // Test Vec4
    const auto& vec4_val = section.find("my_vec4")->second;
    auto* v4_map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&vec4_val.m_value);
    ASSERT_NE(v4_map_ptr, nullptr);
    const auto& v4_map = **v4_map_ptr;
    EXPECT_EQ(std::get<double>(v4_map.find("x")->second.m_value), 10);
    EXPECT_EQ(std::get<double>(v4_map.find("y")->second.m_value), 20);
    EXPECT_EQ(std::get<double>(v4_map.find("z")->second.m_value), 30);
    EXPECT_EQ(std::get<double>(v4_map.find("w")->second.m_value), 40);
}


TEST(DataTypeTest, ThrowsOnIncorrectArgumentCount)
{
    YINI::YiniManager manager;

    // Test Color
    try {
        load_manager_from_source(manager, "test_color_err.yini", "[s]\nk=Color(1,2)");
        FAIL() << "Expected YiniException for Color";
    } catch(const YINI::YiniException& e) {
        EXPECT_STREQ(e.what(), "Color() expects 3 (r, g, b) or 4 (r, g, b, a) arguments.");
    }

    // Test Vec2
    try {
        load_manager_from_source(manager, "test_vec2_err.yini", "[s]\nk=Vec2(1)");
        FAIL() << "Expected YiniException for Vec2";
    } catch(const YINI::YiniException& e) {
        EXPECT_STREQ(e.what(), "Vec2() expects exactly 2 arguments (x, y).");
    }

    // Test Vec3
    try {
        load_manager_from_source(manager, "test_vec3_err.yini", "[s]\nk=Vec3(1,2,3,4)");
        FAIL() << "Expected YiniException for Vec3";
    } catch(const YINI::YiniException& e) {
        EXPECT_STREQ(e.what(), "Vec3() expects exactly 3 arguments (x, y, z).");
    }

    // Test Vec4
    try {
        load_manager_from_source(manager, "test_vec4_err.yini", "[s]\nk=Vec4(1,2,3)");
        FAIL() << "Expected YiniException for Vec4";
    } catch(const YINI::YiniException& e) {
        EXPECT_STREQ(e.what(), "Vec4() expects exactly 4 arguments (x, y, z, w).");
    }
}

TEST(DataTypeTest, ThrowsOnUnknownFunction)
{
    YINI::YiniManager manager;
    try {
        load_manager_from_source(manager, "test_unknown_func.yini", "[s]\nk=UnknownFunc(1,2,3)");
        FAIL() << "Expected YiniException for UnknownFunc";
    } catch(const YINI::YiniException& e) {
        EXPECT_STREQ(e.what(), "Unknown function call 'UnknownFunc'.");
    }
}