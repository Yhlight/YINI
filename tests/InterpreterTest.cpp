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

TEST(InterpreterTest, ThrowsOnDivisionByZero)
{
    std::string source = R"(
        [MySection]
        val = 10 / 0
    )";
    EXPECT_THROW(interpretSource(source), std::runtime_error);
}