#include <gtest/gtest.h>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <vector>
#include <any>

TEST(ParserTest, ParsesSimpleStringValue)
{
    std::string source = "[Section]\nkey = \"value\"";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    ASSERT_NE(sectionNode, nullptr);

    EXPECT_EQ(std::any_cast<std::string>(sectionNode->name.literal), "Section");

    ASSERT_EQ(sectionNode->values.size(), 1);
    auto* keyValueNode = sectionNode->values[0].get();
    ASSERT_NE(keyValueNode, nullptr);

    EXPECT_EQ(std::any_cast<std::string>(keyValueNode->key.literal), "key");

    auto* literalNode = dynamic_cast<YINI::Literal*>(keyValueNode->value.get());
    ASSERT_NE(literalNode, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(literalNode->value), "value");
}

TEST(ParserTest, ParsesVariousDataTypes)
{
    std::string source = R"(
        [Data]
        key_int = 123
        key_float = 3.14
        key_true = true
        key_false = false
    )";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    ASSERT_NE(sectionNode, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode->name.literal), "Data");

    ASSERT_EQ(sectionNode->values.size(), 4);

    // Integer
    auto* kv_int = sectionNode->values[0].get();
    EXPECT_EQ(std::any_cast<std::string>(kv_int->key.literal), "key_int");
    auto* literal_int = dynamic_cast<YINI::Literal*>(kv_int->value.get());
    EXPECT_EQ(std::any_cast<double>(literal_int->value), 123);

    // Float
    auto* kv_float = sectionNode->values[1].get();
    EXPECT_EQ(std::any_cast<std::string>(kv_float->key.literal), "key_float");
    auto* literal_float = dynamic_cast<YINI::Literal*>(kv_float->value.get());
    EXPECT_EQ(std::any_cast<double>(literal_float->value), 3.14);

    // Boolean True
    auto* kv_true = sectionNode->values[2].get();
    EXPECT_EQ(std::any_cast<std::string>(kv_true->key.literal), "key_true");
    auto* literal_true = dynamic_cast<YINI::Literal*>(kv_true->value.get());
    EXPECT_EQ(std::any_cast<bool>(literal_true->value), true);

    // Boolean False
    auto* kv_false = sectionNode->values[3].get();
    EXPECT_EQ(std::any_cast<std::string>(kv_false->key.literal), "key_false");
    auto* literal_false = dynamic_cast<YINI::Literal*>(kv_false->value.get());
    EXPECT_EQ(std::any_cast<bool>(literal_false->value), false);
}