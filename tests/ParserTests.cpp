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
    auto* stmt = dynamic_cast<YINI::KeyValuePairStatement*>(program->statements[0].get());
    ASSERT_NE(stmt, nullptr);

    // Check the key and value
    EXPECT_EQ(stmt->tokenLiteral(), "key");
    EXPECT_EQ(stmt->value.literal, "value");
}