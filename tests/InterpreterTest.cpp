#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Interpreter/Interpreter.h"
#include <vector>
#include <any>

// A helper function to run the interpreter on a given source string.
void interpretSource(const std::string& source) {
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Interpreter interpreter;
    interpreter.interpret(ast);
}

TEST(InterpreterTest, HandlesMacroDefinitionAndResolution)
{
    std::string source = R"(
        [#define]
        my_macro = 123

        [MySection]
        key = @my_macro
    )";

    // This test passes if no exception is thrown, which means the macro
    // was successfully defined and then resolved.
    EXPECT_NO_THROW(interpretSource(source));
}

TEST(InterpreterTest, ThrowsOnUndefinedVariable)
{
    std::string source = R"(
        [MySection]
        key = @undefined_macro
    )";

    // This test expects a runtime_error because the macro is not defined.
    EXPECT_THROW(interpretSource(source), std::runtime_error);
}