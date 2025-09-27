#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include "Lexer/Lexer.h"
#include "Parser/AST.h"
#include <vector>
#include <string>

TEST(ParserTest, ParsesSectionStatement) {
    std::string input = "[MySection]";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();

    // Check for parser errors
    ASSERT_EQ(parser.getErrors().size(), 0);

    // Check that the program has one statement
    ASSERT_EQ(program->statements.size(), 1);

    // Check that the statement is a SectionStatement
    auto* stmt = dynamic_cast<YINI::SectionStatement*>(program->statements[0].get());
    ASSERT_NE(stmt, nullptr);

    // Check the section name
    EXPECT_EQ(stmt->name, "MySection");
}

TEST(ParserTest, ParsesKeyValuePairStatement) {
    std::string input = "key = \"value\"";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();

    // Check for parser errors
    ASSERT_EQ(parser.getErrors().size(), 0);

    // Check that the program has one statement
    ASSERT_EQ(program->statements.size(), 1);

    // Check that the statement is a KeyValuePairStatement
    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    // Check the key
    EXPECT_EQ(kvp_stmt->tokenLiteral(), "key");

    // Check the value
    auto* value_expr = dynamic_cast<YINI::StringLiteral*>(kvp_stmt->value.get());
    ASSERT_NE(value_expr, nullptr);
    EXPECT_EQ(value_expr->value, "value");
}

TEST(ParserTest, ParsesIntegerLiteral) {
    std::string input = "key = 123";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();
    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    auto* int_expr = dynamic_cast<YINI::IntegerLiteral*>(kvp_stmt->value.get());
    ASSERT_NE(int_expr, nullptr);
    EXPECT_EQ(int_expr->value, 123);
}

TEST(ParserTest, ParsesFloatLiteral) {
    std::string input = "key = 3.14";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();
    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    auto* float_expr = dynamic_cast<YINI::FloatLiteral*>(kvp_stmt->value.get());
    ASSERT_NE(float_expr, nullptr);
    EXPECT_DOUBLE_EQ(float_expr->value, 3.14);
}

TEST(ParserTest, ParsesBooleanLiteral) {
    std::string input = "key = true";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();
    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    auto* bool_expr = dynamic_cast<YINI::BooleanLiteral*>(kvp_stmt->value.get());
    ASSERT_NE(bool_expr, nullptr);
    EXPECT_EQ(bool_expr->value, true);
}

TEST(ParserTest, ParsesInfixExpression) {
    std::string input = "key = 5 + 10";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();
    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    auto* infix_expr = dynamic_cast<YINI::InfixExpression*>(kvp_stmt->value.get());
    ASSERT_NE(infix_expr, nullptr);

    auto* left_expr = dynamic_cast<YINI::IntegerLiteral*>(infix_expr->left.get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->value, 5);

    EXPECT_EQ(infix_expr->operator_token.type, YINI::TokenType::Plus);

    auto* right_expr = dynamic_cast<YINI::IntegerLiteral*>(infix_expr->right.get());
    ASSERT_NE(right_expr, nullptr);
    EXPECT_EQ(right_expr->value, 10);
}

// Helper function to test integer literal expressions
void testIntegerLiteral(YINI::Expression* expr, int64_t expected_value) {
    auto* int_expr = dynamic_cast<YINI::IntegerLiteral*>(expr);
    ASSERT_NE(int_expr, nullptr);
    EXPECT_EQ(int_expr->value, expected_value);
}

TEST(ParserTest, ParsesOperatorPrecedence) {
    std::vector<std::pair<std::string, std::string>> tests = {
        {"key = -5 + 10", "((-5) + 10)"}, // Placeholder for when we have prefix ops
        {"key = 5 + 2 * 10", "(5 + (2 * 10))"},
        {"key = 5 * 2 + 10", "((5 * 2) + 10)"},
        {"key = 5 * (2 + 10)", "(5 * (2 + 10))"}, // Placeholder for when we have parens
    };

    // For now, we only test the ones that should work with our current implementation
    std::string input = "key = 5 * 2 + 10";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    // Expected structure: ((5 * 2) + 10)
    auto* outer_infix = dynamic_cast<YINI::InfixExpression*>(kvp_stmt->value.get());
    ASSERT_NE(outer_infix, nullptr);
    EXPECT_EQ(outer_infix->operator_token.type, YINI::TokenType::Plus);
    testIntegerLiteral(outer_infix->right.get(), 10);

    auto* inner_infix = dynamic_cast<YINI::InfixExpression*>(outer_infix->left.get());
    ASSERT_NE(inner_infix, nullptr);
    EXPECT_EQ(inner_infix->operator_token.type, YINI::TokenType::Asterisk);
    testIntegerLiteral(inner_infix->left.get(), 5);
    testIntegerLiteral(inner_infix->right.get(), 2);
}

TEST(ParserTest, ParsesGroupedExpressions) {
    std::string input = "key = (5 + 10) * 2";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    // Expected structure: ((5 + 10) * 2)
    auto* outer_infix = dynamic_cast<YINI::InfixExpression*>(kvp_stmt->value.get());
    ASSERT_NE(outer_infix, nullptr);
    EXPECT_EQ(outer_infix->operator_token.type, YINI::TokenType::Asterisk);
    testIntegerLiteral(outer_infix->right.get(), 2);

    auto* inner_infix = dynamic_cast<YINI::InfixExpression*>(outer_infix->left.get());
    ASSERT_NE(inner_infix, nullptr);
    EXPECT_EQ(inner_infix->operator_token.type, YINI::TokenType::Plus);
    testIntegerLiteral(inner_infix->left.get(), 5);
    testIntegerLiteral(inner_infix->right.get(), 10);
}

TEST(ParserTest, ParsesPrefixExpression) {
    std::string input = "key = -15";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* kvp_stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(kvp_stmt, nullptr);

    auto* prefix_expr = dynamic_cast<YINI::PrefixExpression*>(kvp_stmt->value.get());
    ASSERT_NE(prefix_expr, nullptr);
    EXPECT_EQ(prefix_expr->operator_token.type, YINI::TokenType::Minus);
    testIntegerLiteral(prefix_expr->right.get(), 15);
}

TEST(ParserTest, ParsesQuickRegisterStatement) {
    std::string input = "+= 123";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.getErrors().size(), 0);
    ASSERT_EQ(program->statements.size(), 1);

    auto* stmt = dynamic_cast<YINI::QuickRegisterStatement*>(program->statements[0].get());
    ASSERT_NE(stmt, nullptr);

    EXPECT_EQ(stmt->token.type, YINI::TokenType::PlusAssign);
    testIntegerLiteral(stmt->value.get(), 123);
}