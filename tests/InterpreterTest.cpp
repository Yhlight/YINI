#include <gtest/gtest.h>

#include <fstream>
#include <variant>
#include <vector>

#include "Core/YiniException.h"
#include "Core/YiniManager.h"

// A helper function to create a temporary file and load it with the manager.
static void load_manager_from_source(YINI::YiniManager& manager, const std::string& filename,
                                     const std::string& source) {
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();
    manager.load(filename);
}

TEST(InterpreterTest, HandlesMacroDefinitionAndResolution) {
    std::string source = R"(
        [#define]
        my_macro = 123

        [MySection]
        key = @my_macro
    )";

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_macro.yini", source);
    const auto& interpreter = manager.get_interpreter();
    const auto& section = interpreter.resolved_sections.find("MySection")->second;
    ASSERT_NE(section.find("key"), section.end());
    EXPECT_EQ(std::get<double>(section.find("key")->second.m_value), 123);
}

TEST(InterpreterTest, ThrowsOnUndefinedVariable) {
    std::string filename = "test_undef.yini";
    std::string source = R"(
[MySection]
key = @undefined_macro
)";
    try {
        YINI::YiniManager manager;
        load_manager_from_source(manager, filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 8);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Undefined variable 'undefined_macro'.");
    }
}

TEST(InterpreterTest, EvaluatesArithmeticExpressions) {
    std::string source = R"(
        [#define]
        var = 16
        [MySection]
        val1 = 10 + 2 * 3
        val2 = (10 + 2) * 3
        val3 = -@var + 5
        val4 = 10 % 3
    )";

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_arithmetic.yini", source);
    const auto& interpreter = manager.get_interpreter();
    const auto& section = interpreter.resolved_sections.find("MySection")->second;
    EXPECT_EQ(std::get<double>(section.find("val1")->second.m_value), 16);
    EXPECT_EQ(std::get<double>(section.find("val2")->second.m_value), 36);
    EXPECT_EQ(std::get<double>(section.find("val3")->second.m_value), -11);
    EXPECT_EQ(std::get<double>(section.find("val4")->second.m_value), 1);
}

TEST(InterpreterTest, ThrowsOnTypeMismatch) {
    std::string filename = "test_typemismatch.yini";
    std::string source = R"(
[MySection]
val = 10 + "hello"
)";
    try {
        YINI::YiniManager manager;
        load_manager_from_source(manager, filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 10);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Operands must be numbers for operator '+'.");
    }
}

TEST(InterpreterTest, ThrowsOnDivisionByZero) {
    std::string filename = "test_divzero.yini";
    std::string source = R"(
[MySection]
val = 10 / 0
)";
    try {
        YINI::YiniManager manager;
        load_manager_from_source(manager, filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 3);
        EXPECT_EQ(e.column(), 10);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Division by zero.");
    }
}

TEST(InterpreterTest, EvaluatesDataStructures) {
    std::string source = R"(
        [MySection]
        my_array = [1, "two", 3.0]
        my_set = (1, "two", 3.0)
        my_map = {"a": 1, "b": "two"}
    )";

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_datastructures.yini", source);
    const auto& interpreter = manager.get_interpreter();
    const auto& section = interpreter.resolved_sections.find("MySection")->second;

    // Test Array
    ASSERT_NE(section.find("my_array"), section.end());
    const auto& arr_val = section.find("my_array")->second;
    auto* arr_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&arr_val.m_value);
    ASSERT_NE(arr_ptr, nullptr);
    const auto& arr = **arr_ptr;
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(std::get<double>(arr[0].m_value), 1);
    EXPECT_EQ(std::get<std::string>(arr[1].m_value), "two");
    EXPECT_EQ(std::get<double>(arr[2].m_value), 3.0);

    // Test Set (currently represented as an array)
    ASSERT_NE(section.find("my_set"), section.end());
    const auto& set_val = section.find("my_set")->second;
    auto* set_ptr = std::get_if<std::unique_ptr<YINI::YiniArray>>(&set_val.m_value);
    ASSERT_NE(set_ptr, nullptr);
    const auto& set = **set_ptr;
    ASSERT_EQ(set.size(), 3);
    EXPECT_EQ(std::get<double>(set[0].m_value), 1);
    EXPECT_EQ(std::get<std::string>(set[1].m_value), "two");
    EXPECT_EQ(std::get<double>(set[2].m_value), 3.0);

    // Test Map
    ASSERT_NE(section.find("my_map"), section.end());
    const auto& map_val = section.find("my_map")->second;
    auto* map_ptr = std::get_if<std::unique_ptr<YINI::YiniMap>>(&map_val.m_value);
    ASSERT_NE(map_ptr, nullptr);
    const auto& map = **map_ptr;
    ASSERT_EQ(map.size(), 2);
    EXPECT_EQ(std::get<double>(map.find("a")->second.m_value), 1);
    EXPECT_EQ(std::get<std::string>(map.find("b")->second.m_value), "two");
}

TEST(InterpreterTest, HandlesSectionInheritance) {
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

    YINI::YiniManager manager;
    load_manager_from_source(manager, "test_inheritance.yini", source);
    const auto& interpreter = manager.get_interpreter();

    ASSERT_NE(interpreter.resolved_sections.find("Child"), interpreter.resolved_sections.end());
    const auto& child_section = interpreter.resolved_sections.find("Child")->second;

    EXPECT_EQ(std::get<double>(child_section.find("val1")->second.m_value), 100);  // Child overrides ParentA
    EXPECT_EQ(std::get<std::string>(child_section.find("val2")->second.m_value),
              "overridden");                                                     // ParentB overrides ParentA
    EXPECT_EQ(std::get<double>(child_section.find("val3")->second.m_value), 3);  // Inherited from ParentB
    EXPECT_EQ(std::get<double>(child_section.find("val4")->second.m_value), 4);  // Defined in Child
}

TEST(InterpreterTest, ThrowsOnCircularInheritance) {
    std::string filename = "test_circular.yini";
    std::string source = R"(
[A] : B
[B] : A
)";
    try {
        YINI::YiniManager manager;
        load_manager_from_source(manager, filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 2);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Circular inheritance detected involving section 'A'.");
    }
}

TEST(InterpreterTest, ThrowsOnUndefinedParent) {
    std::string filename = "test_undefparent.yini";
    std::string source = R"(
[A] : NonExistent
)";
    try {
        YINI::YiniManager manager;
        load_manager_from_source(manager, filename, source);
        FAIL() << "Expected RuntimeError";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_EQ(e.line(), 2);
        EXPECT_EQ(e.column(), 7);
        EXPECT_EQ(e.filepath(), filename);
        EXPECT_STREQ(e.what(), "Parent section 'NonExistent' not found.");
    }
}