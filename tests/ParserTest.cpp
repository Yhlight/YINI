#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <vector>
#include <any>

TEST(ParserTest, ParsesSimpleInput)
{
    std::string source = "[Section]\nkey = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::AstNode>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    ASSERT_NE(sectionNode, nullptr);

    EXPECT_EQ(std::any_cast<std::string>(sectionNode->name.literal), "Section");

    ASSERT_EQ(sectionNode->values.size(), 1);
    auto* keyValueNode = sectionNode->values[0].get();
    ASSERT_NE(keyValueNode, nullptr);

    EXPECT_EQ(std::any_cast<std::string>(keyValueNode->key.literal), "key");
    EXPECT_EQ(std::any_cast<std::string>(keyValueNode->value.literal), "value");
}