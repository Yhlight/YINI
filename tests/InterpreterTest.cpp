#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <vector>
#include <any>

// A helper function to create a temporary file, load it with the manager, and return the interpreter.
YINI::Interpreter create_and_load_manager(const std::string& filename, const std::string& source) {
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();

    YINI::YiniManager manager;
    manager.load(filename);
    return manager.interpreter;
}

TEST(InterpreterTest, HandlesMacroDefinitionAndResolution)
{
    std::string source = R"(
        [#define]
        my_macro = 123

        [MySection]
        key = @my_macro
    )";

    auto interpreter = create_and_load_manager("test_macro.yini", source);
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("key"), 1);
    EXPECT_EQ(std::any_cast<double>(interpreter.resolved_sections["MySection"]["key"]), 123);
}

TEST(InterpreterTest, ThrowsOnUndefinedVariable)
{
    std::string source = R"(
        [MySection]
        key = @undefined_macro
    )";

    EXPECT_THROW(create_and_load_manager("test_undef.yini", source), YINI::YiniException);
}

TEST(InterpreterTest, EvaluatesArithmeticExpressions)
{
    std::string source = R"(
        [#define]
        var = 16
        [MySection]
        val1 = 10 + 2 * 3
        val2 = (10 + 2) * 3
        val3 = -@var + 5
        val4 = 10 % 3
    )";

    auto interpreter = create_and_load_manager("test_arithmetic.yini", source);
    EXPECT_EQ(std::any_cast<double>(interpreter.resolved_sections["MySection"]["val1"]), 16);
    EXPECT_EQ(std::any_cast<double>(interpreter.resolved_sections["MySection"]["val2"]), 36);
    EXPECT_EQ(std::any_cast<double>(interpreter.resolved_sections["MySection"]["val3"]), -11);
    EXPECT_EQ(std::any_cast<double>(interpreter.resolved_sections["MySection"]["val4"]), 1);
}

TEST(InterpreterTest, ThrowsOnTypeMismatch)
{
    std::string source = R"(
        [MySection]
        val = 10 + "hello"
    )";
    EXPECT_THROW(create_and_load_manager("test_typemismatch.yini", source), YINI::YiniException);
}

TEST(InterpreterTest, ThrowsOnDivisionByZero)
{
    std::string source = R"(
        [MySection]
        val = 10 / 0
    )";
    EXPECT_THROW(create_and_load_manager("test_divzero.yini", source), YINI::YiniException);
}

TEST(InterpreterTest, EvaluatesDataStructures)
{
    std::string source = R"(
        [MySection]
        my_array = [1, "two", 3.0]
        my_set = (1, "two", 3.0)
        my_map = {"a": 1, "b": "two"}
    )";

    auto interpreter = create_and_load_manager("test_datastructures.yini", source);

    // Test Array
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("my_array"), 1);
    auto arr = std::any_cast<std::vector<std::any>>(interpreter.resolved_sections["MySection"]["my_array"]);
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(std::any_cast<double>(arr[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(arr[1]), "two");
    EXPECT_EQ(std::any_cast<double>(arr[2]), 3.0);

    // Test Set (currently represented as a vector)
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("my_set"), 1);
    auto set = std::any_cast<std::vector<std::any>>(interpreter.resolved_sections["MySection"]["my_set"]);
    ASSERT_EQ(set.size(), 3);
    EXPECT_EQ(std::any_cast<double>(set[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(set[1]), "two");
    EXPECT_EQ(std::any_cast<double>(set[2]), 3.0);

    // Test Map
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("my_map"), 1);
    auto map = std::any_cast<std::map<std::string, std::any>>(interpreter.resolved_sections["MySection"]["my_map"]);
    ASSERT_EQ(map.size(), 2);
    EXPECT_EQ(std::any_cast<double>(map["a"]), 1);
    EXPECT_EQ(std::any_cast<std::string>(map["b"]), "two");
}

TEST(InterpreterTest, HandlesSectionInheritance)
{
    std::string source = R"(
        [ParentA]
        val1 = 1
        val2 = "original"

        [ParentB]
        val2 = "overridden"
        val3 = 3

        [Child] : ParentA, ParentB
        val1 = 100
        val4 = 4
    )";

    auto interpreter = create_and_load_manager("test_inheritance.yini", source);

    ASSERT_EQ(interpreter.resolved_sections.count("Child"), 1);
    const auto& child_section = interpreter.resolved_sections["Child"];

    EXPECT_EQ(std::any_cast<double>(child_section.at("val1")), 100);       // Child overrides ParentA
    EXPECT_EQ(std::any_cast<std::string>(child_section.at("val2")), "overridden"); // ParentB overrides ParentA
    EXPECT_EQ(std::any_cast<double>(child_section.at("val3")), 3);          // Inherited from ParentB
    EXPECT_EQ(std::any_cast<double>(child_section.at("val4")), 4);          // Defined in Child
}

TEST(InterpreterTest, ThrowsOnCircularInheritance)
{
    std::string source = R"(
        [A] : B
        [B] : A
    )";
    EXPECT_THROW(create_and_load_manager("test_circular.yini", source), YINI::YiniException);
}

TEST(InterpreterTest, ThrowsOnUndefinedParent)
{
    std::string source = R"(
        [A] : NonExistent
    )";
    EXPECT_THROW(create_and_load_manager("test_undefparent.yini", source), YINI::YiniException);
}