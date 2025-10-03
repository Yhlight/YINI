#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <vector>
#include <variant>

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
    EXPECT_EQ(std::get<double>(interpreter.resolved_sections["MySection"]["key"].m_value), 123);
}

TEST(InterpreterTest, ThrowsOnUndefinedVariable)
{
    std::string filename = "test_undef.yini";
    std::string source = R"(
[MySection]
key = @undefined_macro
)";
    try {
        create_and_load_manager(filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 8);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Undefined variable 'undefined_macro'.");
    }
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
    EXPECT_EQ(std::get<double>(interpreter.resolved_sections["MySection"]["val1"].m_value), 16);
    EXPECT_EQ(std::get<double>(interpreter.resolved_sections["MySection"]["val2"].m_value), 36);
    EXPECT_EQ(std::get<double>(interpreter.resolved_sections["MySection"]["val3"].m_value), -11);
    EXPECT_EQ(std::get<double>(interpreter.resolved_sections["MySection"]["val4"].m_value), 1);
}

TEST(InterpreterTest, ThrowsOnTypeMismatch)
{
    std::string filename = "test_typemismatch.yini";
    std::string source = R"(
[MySection]
val = 10 + "hello"
)";
    try {
        create_and_load_manager(filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 10);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Operands must be numbers for operator '+'.");
    }
}

TEST(InterpreterTest, ThrowsOnDivisionByZero)
{
    std::string filename = "test_divzero.yini";
    std::string source = R"(
[MySection]
val = 10 / 0
)";
    try {
        create_and_load_manager(filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 10);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Division by zero.");
    }
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
    auto& arr_val = interpreter.resolved_sections["MySection"]["my_array"];
    auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&arr_val.m_value);
    ASSERT_NE(arr_ptr, nullptr);
    const auto& arr = **arr_ptr;
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(std::get<double>(arr[0].m_value), 1);
    EXPECT_EQ(std::get<std::string>(arr[1].m_value), "two");
    EXPECT_EQ(std::get<double>(arr[2].m_value), 3.0);

    // Test Set (currently represented as an array)
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("my_set"), 1);
    auto& set_val = interpreter.resolved_sections["MySection"]["my_set"];
    auto* set_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&set_val.m_value);
    ASSERT_NE(set_ptr, nullptr);
    const auto& set = **set_ptr;
    ASSERT_EQ(set.size(), 3);
    EXPECT_EQ(std::get<double>(set[0].m_value), 1);
    EXPECT_EQ(std::get<std::string>(set[1].m_value), "two");
    EXPECT_EQ(std::get<double>(set[2].m_value), 3.0);

    // Test Map
    ASSERT_EQ(interpreter.resolved_sections["MySection"].count("my_map"), 1);
    auto& map_val = interpreter.resolved_sections["MySection"]["my_map"];
    auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&map_val.m_value);
    ASSERT_NE(map_ptr, nullptr);
    const auto& map = **map_ptr;
    ASSERT_EQ(map.size(), 2);
    EXPECT_EQ(std::get<double>(map.at("a").m_value), 1);
    EXPECT_EQ(std::get<std::string>(map.at("b").m_value), "two");
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

    EXPECT_EQ(std::get<double>(child_section.at("val1").m_value), 100);       // Child overrides ParentA
    EXPECT_EQ(std::get<std::string>(child_section.at("val2").m_value), "overridden"); // ParentB overrides ParentA
    EXPECT_EQ(std::get<double>(child_section.at("val3").m_value), 3);          // Inherited from ParentB
    EXPECT_EQ(std::get<double>(child_section.at("val4").m_value), 4);          // Defined in Child
}

TEST(InterpreterTest, HandlesMultiLevelInheritance)
{
    std::string source = R"(
        [A]
        keyA = "from A"
        keyB = "from A"
        keyC = "from A"

        [B] : A
        keyB = "from B"
        keyC = "from B"

        [C] : B
        keyC = "from C"
    )";

    auto interpreter = create_and_load_manager("test_multilevel_inheritance.yini", source);

    ASSERT_EQ(interpreter.resolved_sections.count("C"), 1);
    const auto& child_section = interpreter.resolved_sections["C"];

    EXPECT_EQ(std::get<std::string>(child_section.at("keyA").m_value), "from A"); // Inherited from A
    EXPECT_EQ(std::get<std::string>(child_section.at("keyB").m_value), "from B"); // Inherited from B (overrides A)
    EXPECT_EQ(std::get<std::string>(child_section.at("keyC").m_value), "from C"); // Defined in C (overrides B)
}

TEST(InterpreterTest, ThrowsOnCircularInheritance)
{
    std::string filename = "test_circular.yini";
    std::string source = R"(
[A] : B
[B] : A
)";
    try {
        create_and_load_manager(filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 2);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Circular inheritance detected involving section 'A'.");
    }
}

TEST(InterpreterTest, ThrowsOnUndefinedParent)
{
    std::string filename = "test_undefparent.yini";
    std::string source = R"(
[A] : NonExistent
)";
    try {
        create_and_load_manager(filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 2);
        EXPECT_EQ(e.column(), 7);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Parent section 'NonExistent' not found.");
    }
}