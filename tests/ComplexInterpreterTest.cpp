#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <variant>

// Helper from InterpreterTest
YINI::Interpreter create_and_load_manager_for_complex(const std::string& filename, const std::string& source) {
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();

    YINI::YiniManager manager;
    manager.load(filename);
    return manager.get_interpreter();
}

TEST(ComplexInterpreterTest, HandlesDiamondInheritanceCorrectly)
{
    std::string source = R"(
        [Base]
        value = "from base"

        [DerivedA] : Base
        value = "from derived A"

        [DerivedB] : Base
        value = "from derived B"

        // Order matters: DerivedB is last, so it should win the override race.
        [Child] : DerivedA, DerivedB
    )";

    auto interpreter = create_and_load_manager_for_complex("test_diamond.yini", source);
    const auto& child_section = interpreter.resolved_sections["Child"];

    ASSERT_EQ(child_section.count("value"), 1);
    EXPECT_EQ(std::get<std::string>(child_section.at("value").m_value), "from derived B");
}

TEST(ComplexInterpreterTest, HandlesDeeplyNestedIncludes)
{
    // Create the file chain: root -> one -> two -> three
    std::ofstream("nested_three.yini") << "[Deep]\nkey = \"deepest\"";
    std::ofstream("nested_two.yini") << "[#include]\n+= \"nested_three.yini\"\n[Level2]\nkey = \"two\"";
    std::ofstream("nested_one.yini") << "[#include]\n+= \"nested_two.yini\"\n[Level1]\nkey = \"one\"";
    std::ofstream("nested_root.yini") << "[#include]\n+= \"nested_one.yini\"\n[Root]\nkey = \"root\"";

    YINI::YiniManager manager;
    manager.load("nested_root.yini");

    const auto& resolved = manager.get_interpreter().resolved_sections;

    // Check that all sections from all files have been loaded correctly.
    ASSERT_EQ(resolved.count("Root"), 1);
    ASSERT_EQ(resolved.count("Level1"), 1);
    ASSERT_EQ(resolved.count("Level2"), 1);
    ASSERT_EQ(resolved.count("Deep"), 1);

    EXPECT_EQ(std::get<std::string>(resolved.at("Root").at("key").m_value), "root");
    EXPECT_EQ(std::get<std::string>(resolved.at("Level1").at("key").m_value), "one");
    EXPECT_EQ(std::get<std::string>(resolved.at("Level2").at("key").m_value), "two");
    EXPECT_EQ(std::get<std::string>(resolved.at("Deep").at("key").m_value), "deepest");
}