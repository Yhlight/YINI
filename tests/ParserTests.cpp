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