#include <gtest/gtest.h>
#include "Parser.h"
#include "Lexer.h"
#include "Ast.h"

TEST(ParserTest, TestSection)
{
    std::string source = "[Config]";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::SectionStmt* section = dynamic_cast<YINI::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Config");
    EXPECT_TRUE(section->inheritance.empty());
}

TEST(ParserTest, TestKeyValue)
{
    std::string source = "key = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);
    EXPECT_EQ(kv->key.lexeme, "key");

    YINI::LiteralExpr* literal = dynamic_cast<YINI::LiteralExpr*>(kv->value.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(std::get<std::string>(literal->token.literal), "value");
}

TEST(ParserTest, TestSectionInheritance)
{
    std::string source = "[Config3] : Config1, Config2";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::SectionStmt* section = dynamic_cast<YINI::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Config3");

    ASSERT_EQ(section->inheritance.size(), 2);
    EXPECT_EQ(section->inheritance[0].lexeme, "Config1");
    EXPECT_EQ(section->inheritance[1].lexeme, "Config2");
}

TEST(ParserTest, TestArithmeticExpression)
{
    std::string source = "key = 1 + 2 * 3";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);

    // AST check for: 1 + (2 * 3)
    YINI::BinaryExpr* add_expr = dynamic_cast<YINI::BinaryExpr*>(kv->value.get());
    ASSERT_NE(add_expr, nullptr);
    EXPECT_EQ(add_expr->op.type, YINI::TokenType::Plus);

    YINI::LiteralExpr* literal1 = dynamic_cast<YINI::LiteralExpr*>(add_expr->left.get());
    ASSERT_NE(literal1, nullptr);
    EXPECT_EQ(std::get<long long>(literal1->token.literal), 1);

    YINI::BinaryExpr* mul_expr = dynamic_cast<YINI::BinaryExpr*>(add_expr->right.get());
    ASSERT_NE(mul_expr, nullptr);
    EXPECT_EQ(mul_expr->op.type, YINI::TokenType::Star);

    YINI::LiteralExpr* literal2 = dynamic_cast<YINI::LiteralExpr*>(mul_expr->left.get());
    ASSERT_NE(literal2, nullptr);
    EXPECT_EQ(std::get<long long>(literal2->token.literal), 2);

    YINI::LiteralExpr* literal3 = dynamic_cast<YINI::LiteralExpr*>(mul_expr->right.get());
    ASSERT_NE(literal3, nullptr);
    EXPECT_EQ(std::get<long long>(literal3->token.literal), 3);
}

TEST(ParserTest, TestArrayExpression)
{
    std::string source = "key = [1, \"two\", 3.14, true]";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);

    YINI::ArrayExpr* array_expr = dynamic_cast<YINI::ArrayExpr*>(kv->value.get());
    ASSERT_NE(array_expr, nullptr);
    ASSERT_EQ(array_expr->elements.size(), 4);

    // Check element 1
    YINI::LiteralExpr* elem1 = dynamic_cast<YINI::LiteralExpr*>(array_expr->elements[0].get());
    ASSERT_NE(elem1, nullptr);
    EXPECT_EQ(std::get<long long>(elem1->token.literal), 1);

    // Check element 2
    YINI::LiteralExpr* elem2 = dynamic_cast<YINI::LiteralExpr*>(array_expr->elements[1].get());
    ASSERT_NE(elem2, nullptr);
    EXPECT_EQ(std::get<std::string>(elem2->token.literal), "two");

    // Check element 3
    YINI::LiteralExpr* elem3 = dynamic_cast<YINI::LiteralExpr*>(array_expr->elements[2].get());
    ASSERT_NE(elem3, nullptr);
    EXPECT_EQ(std::get<double>(elem3->token.literal), 3.14);

    // Check element 4
    YINI::LiteralExpr* elem4 = dynamic_cast<YINI::LiteralExpr*>(array_expr->elements[3].get());
    ASSERT_NE(elem4, nullptr);
    EXPECT_EQ(elem4->token.type, YINI::TokenType::True);
}

TEST(ParserTest, TestCallExpression)
{
    std::string source = "key = Coord(1, 2)";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);

    YINI::CallExpr* call_expr = dynamic_cast<YINI::CallExpr*>(kv->value.get());
    ASSERT_NE(call_expr, nullptr);

    YINI::LiteralExpr* callee = dynamic_cast<YINI::LiteralExpr*>(call_expr->callee.get());
    ASSERT_NE(callee, nullptr);
    EXPECT_EQ(callee->token.lexeme, "Coord");

    ASSERT_EQ(call_expr->arguments.size(), 2);

    YINI::LiteralExpr* arg1 = dynamic_cast<YINI::LiteralExpr*>(call_expr->arguments[0].get());
    ASSERT_NE(arg1, nullptr);
    EXPECT_EQ(std::get<long long>(arg1->token.literal), 1);

    YINI::LiteralExpr* arg2 = dynamic_cast<YINI::LiteralExpr*>(call_expr->arguments[1].get());
    ASSERT_NE(arg2, nullptr);
    EXPECT_EQ(std::get<long long>(arg2->token.literal), 2);
}

TEST(ParserTest, TestDefineSection)
{
    std::string source = "[#define]\nname = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::SectionStmt* section = dynamic_cast<YINI::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "#define");

    ASSERT_EQ(section->statements.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv, nullptr);
    EXPECT_EQ(kv->key.lexeme, "name");

    YINI::LiteralExpr* literal = dynamic_cast<YINI::LiteralExpr*>(kv->value.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(std::get<std::string>(literal->token.literal), "value");
}

TEST(ParserTest, TestIncludeSection)
{
    std::string source = "[#include]\n+= \"file1.yini\"\n+= \"file2.yini\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::SectionStmt* section = dynamic_cast<YINI::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "#include");

    ASSERT_EQ(section->statements.size(), 2);

    YINI::RegisterStmt* reg1 = dynamic_cast<YINI::RegisterStmt*>(section->statements[0].get());
    ASSERT_NE(reg1, nullptr);
    YINI::LiteralExpr* val1 = dynamic_cast<YINI::LiteralExpr*>(reg1->value.get());
    ASSERT_NE(val1, nullptr);
    EXPECT_EQ(std::get<std::string>(val1->token.literal), "file1.yini");

    YINI::RegisterStmt* reg2 = dynamic_cast<YINI::RegisterStmt*>(section->statements[1].get());
    ASSERT_NE(reg2, nullptr);
    YINI::LiteralExpr* val2 = dynamic_cast<YINI::LiteralExpr*>(reg2->value.get());
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(std::get<std::string>(val2->token.literal), "file2.yini");
}

TEST(ParserTest, TestMapExpression)
{
    std::string source = "data = {key1: \"value1\", key2: 2}";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);

    YINI::MapExpr* map_expr = dynamic_cast<YINI::MapExpr*>(kv->value.get());
    ASSERT_NE(map_expr, nullptr);
    ASSERT_EQ(map_expr->pairs.size(), 2);

    // Check first pair
    YINI::KeyValuePairExpr* pair1 = map_expr->pairs[0].get();
    ASSERT_NE(pair1, nullptr);
    EXPECT_EQ(pair1->key.lexeme, "key1");
    YINI::LiteralExpr* val1 = dynamic_cast<YINI::LiteralExpr*>(pair1->value.get());
    ASSERT_NE(val1, nullptr);
    EXPECT_EQ(std::get<std::string>(val1->token.literal), "value1");

    // Check second pair
    YINI::KeyValuePairExpr* pair2 = map_expr->pairs[1].get();
    ASSERT_NE(pair2, nullptr);
    EXPECT_EQ(pair2->key.lexeme, "key2");
    YINI::LiteralExpr* val2 = dynamic_cast<YINI::LiteralExpr*>(pair2->value.get());
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(std::get<long long>(val2->token.literal), 2);
}

TEST(ParserTest, TestDynaExpression)
{
    std::string source = "key = Dyna(123)";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::KeyValueStmt* kv = dynamic_cast<YINI::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(kv, nullptr);

    YINI::DynaExpr* dyna_expr = dynamic_cast<YINI::DynaExpr*>(kv->value.get());
    ASSERT_NE(dyna_expr, nullptr);

    YINI::LiteralExpr* literal = dynamic_cast<YINI::LiteralExpr*>(dyna_expr->expression.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(std::get<long long>(literal->token.literal), 123);
}

TEST(ParserTest, TestRegisterStatement)
{
    std::string source = "[Reg]\n+= value1\n+= 2";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    YINI::SectionStmt* section = dynamic_cast<YINI::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    // Check first register statement
    YINI::RegisterStmt* reg1 = dynamic_cast<YINI::RegisterStmt*>(section->statements[0].get());
    ASSERT_NE(reg1, nullptr);
    EXPECT_EQ(reg1->op.type, YINI::TokenType::PlusEquals);
    YINI::LiteralExpr* val1 = dynamic_cast<YINI::LiteralExpr*>(reg1->value.get());
    ASSERT_NE(val1, nullptr);
    EXPECT_EQ(val1->token.lexeme, "value1");

    // Check second register statement
    YINI::RegisterStmt* reg2 = dynamic_cast<YINI::RegisterStmt*>(section->statements[1].get());
    ASSERT_NE(reg2, nullptr);
    EXPECT_EQ(reg2->op.type, YINI::TokenType::PlusEquals);
    YINI::LiteralExpr* val2 = dynamic_cast<YINI::LiteralExpr*>(reg2->value.get());
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(std::get<long long>(val2->token.literal), 2);
}