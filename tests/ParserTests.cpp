#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/AST.h"

TEST(ParserTests, ParsesSectionWithKeyValue)
{
    std::string source = "[TestSection]\nkey = \"value\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "TestSection");

    ASSERT_EQ(section->statements.size(), 1);

    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);
    EXPECT_EQ(keyValue->key.lexeme, "key");

    auto literal = dynamic_cast<YINI::AST::LiteralExpr*>(keyValue->value.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(std::get<std::string>(literal->value.literal), "value");
}

TEST(ParserTests, ParsesNumberLiteral)
{
    std::string source = "[Numbers]\nvalue = 123";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);
    auto literal = dynamic_cast<YINI::AST::LiteralExpr*>(keyValue->value.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(std::get<double>(literal->value.literal), 123);
}

TEST(ParserTests, ThrowsErrorOnMissingBracket)
{
    std::string source = "[TestSection";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);

    EXPECT_THROW(parser.parse(), std::runtime_error);
}

TEST(ParserTests, ParsesBooleanLiterals)
{
    std::string source = "[Booleans]\ntrue_val = true\nfalse_val = false";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    auto true_kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(true_kv, nullptr);
    EXPECT_EQ(true_kv->key.lexeme, "true_val");
    auto true_expr = dynamic_cast<YINI::AST::BoolExpr*>(true_kv->value.get());
    ASSERT_NE(true_expr, nullptr);
    EXPECT_EQ(true_expr->value, true);

    auto false_kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[1].get());
    ASSERT_NE(false_kv, nullptr);
    EXPECT_EQ(false_kv->key.lexeme, "false_val");
    auto false_expr = dynamic_cast<YINI::AST::BoolExpr*>(false_kv->value.get());
    ASSERT_NE(false_expr, nullptr);
    EXPECT_EQ(false_expr->value, false);
}

TEST(ParserTests, ParsesArrayOfNumbers)
{
    std::string source = "[Arrays]\nnumbers = [1, 2, 3]";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto array_expr = dynamic_cast<YINI::AST::ArrayExpr*>(keyValue->value.get());
    ASSERT_NE(array_expr, nullptr);
    ASSERT_EQ(array_expr->elements.size(), 3);

    auto e1 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[0].get());
    ASSERT_NE(e1, nullptr);
    EXPECT_EQ(std::get<double>(e1->value.literal), 1);

    auto e2 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[1].get());
    ASSERT_NE(e2, nullptr);
    EXPECT_EQ(std::get<double>(e2->value.literal), 2);

    auto e3 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[2].get());
    ASSERT_NE(e3, nullptr);
    EXPECT_EQ(std::get<double>(e3->value.literal), 3);
}

TEST(ParserTests, ParsesEmptyArray)
{
    std::string source = "[Arrays]\nempty = []";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto array_expr = dynamic_cast<YINI::AST::ArrayExpr*>(keyValue->value.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements.size(), 0);
}

TEST(ParserTests, ParsesSectionWithSingleInheritance)
{
    std::string source = "[Child : Parent]";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Child");
    ASSERT_EQ(section->parent_sections.size(), 1);
    EXPECT_EQ(section->parent_sections[0].lexeme, "Parent");
}

TEST(ParserTests, ParsesSectionWithMultipleInheritance)
{
    std::string source = "[Child : Parent1, Parent2,Parent3]";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Child");
    ASSERT_EQ(section->parent_sections.size(), 3);
    EXPECT_EQ(section->parent_sections[0].lexeme, "Parent1");
    EXPECT_EQ(section->parent_sections[1].lexeme, "Parent2");
    EXPECT_EQ(section->parent_sections[2].lexeme, "Parent3");
}