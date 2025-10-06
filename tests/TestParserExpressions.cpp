#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

// Helper function to test an integer literal expression
void testIntegerLiteral(const std::shared_ptr<YINI::Expression>& expr, int64_t expected_value) {
    auto int_lit = std::dynamic_pointer_cast<YINI::IntegerLiteral>(expr);
    ASSERT_NE(int_lit, nullptr);
    ASSERT_EQ(int_lit->value, expected_value);
}

// Helper function to test an infix expression
void testInfixExpression(const std::shared_ptr<YINI::Expression>& expr,
                         const std::shared_ptr<YINI::Expression>& expected_left,
                         const std::string& expected_operator,
                         const std::shared_ptr<YINI::Expression>& expected_right) {
    auto infix_expr = std::dynamic_pointer_cast<YINI::InfixExpression>(expr);
    ASSERT_NE(infix_expr, nullptr);
    // This is a simplified test, we'll need to expand this to handle different types
    // For now, we assume integer literals for left/right
    testIntegerLiteral(infix_expr->left, std::dynamic_pointer_cast<YINI::IntegerLiteral>(expected_left)->value);
    ASSERT_EQ(infix_expr->op, expected_operator);
    testIntegerLiteral(infix_expr->right, std::dynamic_pointer_cast<YINI::IntegerLiteral>(expected_right)->value);
}


TEST(ParserExpressionsTest, OperatorPrecedence) {
    std::string input = R"(
[TestSection]
key = 1 + 2 * 3
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto stmt = program->statements[0];
    auto section = std::dynamic_pointer_cast<YINI::Section>(stmt);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->pairs.size(), 1);

    auto pair = section->pairs[0];
    auto expr = pair->value;

    // The AST should represent (1 + (2 * 3))
    auto add_expr = std::dynamic_pointer_cast<YINI::InfixExpression>(expr);
    ASSERT_NE(add_expr, nullptr);
    ASSERT_EQ(add_expr->op, "+");

    testIntegerLiteral(add_expr->left, 1);

    auto mul_expr = std::dynamic_pointer_cast<YINI::InfixExpression>(add_expr->right);
    ASSERT_NE(mul_expr, nullptr);
    ASSERT_EQ(mul_expr->op, "*");

    testIntegerLiteral(mul_expr->left, 2);
    testIntegerLiteral(mul_expr->right, 3);
}

TEST(ParserExpressionsTest, LiteralExpressions) {
    std::string input = R"(
[Literals]
float_val = 3.14
bool_val_true = true
bool_val_false = false
string_val = "hello world"
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->pairs.size(), 4);

    // Test float
    auto float_pair = section->pairs[0];
    ASSERT_EQ(float_pair->key->value, "float_val");
    auto float_lit = std::dynamic_pointer_cast<YINI::FloatLiteral>(float_pair->value);
    ASSERT_NE(float_lit, nullptr);
    ASSERT_FLOAT_EQ(float_lit->value, 3.14);

    // Test true boolean
    auto bool_true_pair = section->pairs[1];
    ASSERT_EQ(bool_true_pair->key->value, "bool_val_true");
    auto bool_true_lit = std::dynamic_pointer_cast<YINI::BooleanLiteral>(bool_true_pair->value);
    ASSERT_NE(bool_true_lit, nullptr);
    ASSERT_EQ(bool_true_lit->value, true);

    // Test false boolean
    auto bool_false_pair = section->pairs[2];
    ASSERT_EQ(bool_false_pair->key->value, "bool_val_false");
    auto bool_false_lit = std::dynamic_pointer_cast<YINI::BooleanLiteral>(bool_false_pair->value);
    ASSERT_NE(bool_false_lit, nullptr);
    ASSERT_EQ(bool_false_lit->value, false);

    // Test string
    auto string_pair = section->pairs[3];
    ASSERT_EQ(string_pair->key->value, "string_val");
    auto string_lit = std::dynamic_pointer_cast<YINI::StringLiteral>(string_pair->value);
    ASSERT_NE(string_lit, nullptr);
    ASSERT_EQ(string_lit->value, "\"hello world\""); // Lexer includes quotes, parser will strip them later if needed
}

TEST(ParserExpressionsTest, ArrayLiteralParsing) {
    std::string input = R"(
[Arrays]
my_array = [1, true, "three"]
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);

    auto pair = section->pairs[0];
    auto array_lit = std::dynamic_pointer_cast<YINI::ArrayLiteral>(pair->value);
    ASSERT_NE(array_lit, nullptr);
    ASSERT_EQ(array_lit->elements.size(), 3);

    // Test first element: Integer
    auto int_elem = std::dynamic_pointer_cast<YINI::IntegerLiteral>(array_lit->elements[0]);
    ASSERT_NE(int_elem, nullptr);
    ASSERT_EQ(int_elem->value, 1);

    // Test second element: Boolean
    auto bool_elem = std::dynamic_pointer_cast<YINI::BooleanLiteral>(array_lit->elements[1]);
    ASSERT_NE(bool_elem, nullptr);
    ASSERT_EQ(bool_elem->value, true);

    // Test third element: String
    auto str_elem = std::dynamic_pointer_cast<YINI::StringLiteral>(array_lit->elements[2]);
    ASSERT_NE(str_elem, nullptr);
    ASSERT_EQ(str_elem->value, "\"three\"");
}