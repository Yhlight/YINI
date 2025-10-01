#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/AstPrinter.h"
#include <vector>
#include <any>

std::unique_ptr<YINI::Expr> parseExpression(const std::string& source) {
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    // Manually create a dummy key-value structure for the parser.
    std::vector<YINI::Token> testTokens;
    testTokens.push_back({YINI::TokenType::LEFT_BRACKET, "[", std::any{}, 1});
    testTokens.push_back({YINI::TokenType::IDENTIFIER, "dummy", std::any{}, 1});
    testTokens.push_back({YINI::TokenType::RIGHT_BRACKET, "]", std::any{}, 1});
    testTokens.push_back({YINI::TokenType::IDENTIFIER, "key", std::any{}, 1});
    testTokens.push_back({YINI::TokenType::EQUAL, "=", std::any{}, 1});
    testTokens.insert(testTokens.end(), tokens.begin(), tokens.end() - 1); // Exclude EOF
    testTokens.push_back({YINI::TokenType::END_OF_FILE, "", std::any{}, 1});

    YINI::Parser parser(testTokens);
    auto ast = parser.parse();
    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    return std::move(sectionNode->values[0]->value);
}


TEST(ExpressionTest, ParsesArithmeticExpressions)
{
    auto expr = parseExpression("-1 + 2 * 3");
    YINI::AstPrinter printer;
    EXPECT_EQ(printer.print(*expr), "(+ (- 1) (* 2 3))");

    auto expr2 = parseExpression("(1 + 2) * 3");
    EXPECT_EQ(printer.print(*expr2), "(* (group (+ 1 2)) 3)");
}

TEST(ExpressionTest, ParsesArrayLiterals)
{
    YINI::AstPrinter printer;

    // Empty array
    auto expr1 = parseExpression("[]");
    EXPECT_EQ(printer.print(*expr1), "(array)");

    // Single-element array
    auto expr2 = parseExpression("[1]");
    EXPECT_EQ(printer.print(*expr2), "(array 1)");

    // Multi-element array
    auto expr3 = parseExpression("[1, \"hello\", true]");
    EXPECT_EQ(printer.print(*expr3), "(array 1 hello true)");
}

TEST(ExpressionTest, ParsesMapAndSetLiterals)
{
    YINI::AstPrinter printer;

    // Empty set
    auto expr1 = parseExpression("()");
    EXPECT_EQ(printer.print(*expr1), "(set)");

    // Single-element set
    auto expr2 = parseExpression("(1,)");
    EXPECT_EQ(printer.print(*expr2), "(set 1)");

    // Multi-element set
    auto expr3 = parseExpression("(1, \"two\")");
    EXPECT_EQ(printer.print(*expr3), "(set 1 two)");

    // Empty map
    auto expr4 = parseExpression("{}");
    EXPECT_EQ(printer.print(*expr4), "(map)");

    // Simple map
    auto expr5 = parseExpression("{\"key\": \"value\"}");
    EXPECT_EQ(printer.print(*expr5), "(map (key value))");

    // Complex map
    auto expr6 = parseExpression("{\"key1\": 1, \"key2\": [1, 2]}");
    EXPECT_EQ(printer.print(*expr6), "(map (key1 1) (key2 (array 1 2)))");
}

TEST(ExpressionTest, ParsesCallExpressions)
{
    YINI::AstPrinter printer;

    // No arguments
    auto expr1 = parseExpression("Dyna()");
    EXPECT_EQ(printer.print(*expr1), "(call Dyna)");

    // Single argument
    auto expr2 = parseExpression("Path(\"a/b\")");
    EXPECT_EQ(printer.print(*expr2), "(call Path a/b)");

    // Multiple arguments
    auto expr3 = parseExpression("Color(255, 192, 203)");
    EXPECT_EQ(printer.print(*expr3), "(call Color 255 192 203)");
}