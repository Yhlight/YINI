#include <gtest/gtest.h>

#include <variant>
#include <vector>

#include "Lexer/Lexer.h"
#include "Parser/AstPrinter.h"
#include "Parser/Parser.h"

std::unique_ptr<YINI::Expr> parseExpression(const std::string& source) {
    YINI::Lexer lexer(source, "test_expr.yini");
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    // Manually create a dummy key-value structure for the parser.
    std::vector<YINI::Token> testTokens;
    testTokens.push_back({YINI::TokenType::LEFT_BRACKET, "[", {}, 1, 1, "test_expr.yini"});
    testTokens.push_back({YINI::TokenType::IDENTIFIER, "dummy", {}, 1, 1, "test_expr.yini"});
    testTokens.push_back({YINI::TokenType::RIGHT_BRACKET, "]", {}, 1, 1, "test_expr.yini"});
    testTokens.push_back({YINI::TokenType::IDENTIFIER, "key", {}, 1, 1, "test_expr.yini"});
    testTokens.push_back({YINI::TokenType::EQUAL, "=", {}, 1, 1, "test_expr.yini"});
    testTokens.insert(testTokens.end(), tokens.begin(), tokens.end() - 1);  // Exclude EOF
    testTokens.push_back({YINI::TokenType::END_OF_FILE, "", {}, 1, 1, "test_expr.yini"});

    YINI::Parser parser(testTokens);
    auto ast = parser.parse();
    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    auto* keyValueNode = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[0].get());
    return std::move(keyValueNode->value);
}

TEST(ExpressionTest, ParsesArithmeticExpressions) {
    auto expr = parseExpression("-1 + 2 * 3");
    YINI::AstPrinter printer;
    EXPECT_EQ(printer.print(*expr), "(+ (- 1) (* 2 3))");

    auto expr2 = parseExpression("(1 + 2) * 3");
    EXPECT_EQ(printer.print(*expr2), "(* (group (+ 1 2)) 3)");
}

TEST(ExpressionTest, ParsesArrayLiterals) {
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

TEST(ExpressionTest, ParsesMapAndSetLiterals) {
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

TEST(ExpressionTest, ParsesVariableExpression) {
    YINI::AstPrinter printer;
    auto expr = parseExpression("@my_var");
    EXPECT_EQ(printer.print(*expr), "my_var");
}

TEST(ExpressionTest, ParsesCallExpressions) {
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