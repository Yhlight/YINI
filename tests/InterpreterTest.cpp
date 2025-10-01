#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Interpreter/Interpreter.h"
#include <vector>
#include <any>

// A helper function to run the interpreter on a given source string and return the interpreter.
YINI::Interpreter interpretSource(const std::string& source) {
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Interpreter interpreter;
    interpreter.interpret(ast);
    return interpreter;
}

TEST(InterpreterTest, HandlesMacroDefinitionAndResolution)
{
    std::string source = R"(
        [#define]
        my_macro = 123

        [MySection]
        key = @my_macro
    )";

    auto interpreter = interpretSource(source);
    ASSERT_EQ(interpreter.results.count("key"), 1);
    EXPECT_EQ(std::any_cast<double>(interpreter.results["key"]), 123);
}

TEST(InterpreterTest, ThrowsOnUndefinedVariable)
{
    std::string source = R"(
        [MySection]
        key = @undefined_macro
    )";

    EXPECT_THROW(interpretSource(source), std::runtime_error);
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

    auto interpreter = interpretSource(source);
    EXPECT_EQ(std::any_cast<double>(interpreter.results["val1"]), 16);
    EXPECT_EQ(std::any_cast<double>(interpreter.results["val2"]), 36);
    EXPECT_EQ(std::any_cast<double>(interpreter.results["val3"]), -11);
    EXPECT_EQ(std::any_cast<double>(interpreter.results["val4"]), 1);
}

TEST(InterpreterTest, ThrowsOnTypeMismatch)
{
    std::string source = R"(
        [MySection]
        val = 10 + "hello"
    )";
    EXPECT_THROW(interpretSource(source), std::runtime_error);
}

TEST(InterpreterTest, EvaluatesDataStructures)
{
    std::string source = R"(
        [MySection]
        my_array = [1, "two", 3.0]
        my_set = (1, "two", 3.0)
        my_map = {"a": 1, "b": "two"}
    )";

    auto interpreter = interpretSource(source);

    // Test Array
    ASSERT_EQ(interpreter.results.count("my_array"), 1);
    auto arr = std::any_cast<std::vector<std::any>>(interpreter.results["my_array"]);
    ASSERT_EQ(arr.size(), 3);
    EXPECT_EQ(std::any_cast<double>(arr[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(arr[1]), "two");
    EXPECT_EQ(std::any_cast<double>(arr[2]), 3.0);

    // Test Set (currently represented as a vector)
    ASSERT_EQ(interpreter.results.count("my_set"), 1);
    auto set = std::any_cast<std::vector<std::any>>(interpreter.results["my_set"]);
    ASSERT_EQ(set.size(), 3);
    EXPECT_EQ(std::any_cast<double>(set[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(set[1]), "two");
    EXPECT_EQ(std::any_cast<double>(set[2]), 3.0);

    // Test Map
    ASSERT_EQ(interpreter.results.count("my_map"), 1);
    auto map = std::any_cast<std::map<std::string, std::any>>(interpreter.results["my_map"]);
    ASSERT_EQ(map.size(), 2);
    EXPECT_EQ(std::any_cast<double>(map["a"]), 1);
    EXPECT_EQ(std::any_cast<std::string>(map["b"]), "two");
}

TEST(InterpreterTest, ThrowsOnDivisionByZero)
{
    std::string source = R"(
        [MySection]
        val = 10 / 0
    )";
    EXPECT_THROW(interpretSource(source), std::runtime_error);
}