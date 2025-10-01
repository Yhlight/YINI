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

    ASSERT_EQ(sectionNode->statements.size(), 1);
    auto* keyValueNode = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[0].get());
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

    ASSERT_EQ(sectionNode->statements.size(), 4);

    // Integer
    auto* kv_int = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[0].get());
    EXPECT_EQ(std::any_cast<std::string>(kv_int->key.literal), "key_int");
    auto* literal_int = dynamic_cast<YINI::Literal*>(kv_int->value.get());
    EXPECT_EQ(std::any_cast<double>(literal_int->value), 123);

    // Float
    auto* kv_float = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[1].get());
    EXPECT_EQ(std::any_cast<std::string>(kv_float->key.literal), "key_float");
    auto* literal_float = dynamic_cast<YINI::Literal*>(kv_float->value.get());
    EXPECT_EQ(std::any_cast<double>(literal_float->value), 3.14);

    // Boolean True
    auto* kv_true = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[2].get());
    EXPECT_EQ(std::any_cast<std::string>(kv_true->key.literal), "key_true");
    auto* literal_true = dynamic_cast<YINI::Literal*>(kv_true->value.get());
    EXPECT_EQ(std::any_cast<bool>(literal_true->value), true);

    // Boolean False
    auto* kv_false = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[3].get());
    EXPECT_EQ(std::any_cast<std::string>(kv_false->key.literal), "key_false");
    auto* literal_false = dynamic_cast<YINI::Literal*>(kv_false->value.get());
    EXPECT_EQ(std::any_cast<bool>(literal_false->value), false);
}

TEST(ParserTest, ParsesSectionInheritance)
{
    // No inheritance
    std::string source1 = "[SectionA]";
    YINI::Lexer lexer1(source1);
    std::vector<YINI::Token> tokens1 = lexer1.scanTokens();
    YINI::Parser parser1(tokens1);
    std::vector<std::unique_ptr<YINI::Stmt>> ast1 = parser1.parse();
    ASSERT_EQ(ast1.size(), 1);
    auto* sectionNode1 = dynamic_cast<YINI::Section*>(ast1[0].get());
    ASSERT_NE(sectionNode1, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode1->name.literal), "SectionA");
    EXPECT_EQ(sectionNode1->parents.size(), 0);

    // Single inheritance
    std::string source2 = "[SectionB] : ParentA";
    YINI::Lexer lexer2(source2);
    std::vector<YINI::Token> tokens2 = lexer2.scanTokens();
    YINI::Parser parser2(tokens2);
    std::vector<std::unique_ptr<YINI::Stmt>> ast2 = parser2.parse();
    ASSERT_EQ(ast2.size(), 1);
    auto* sectionNode2 = dynamic_cast<YINI::Section*>(ast2[0].get());
    ASSERT_NE(sectionNode2, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode2->name.literal), "SectionB");
    ASSERT_EQ(sectionNode2->parents.size(), 1);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode2->parents[0].literal), "ParentA");

    // Multiple inheritance
    std::string source3 = "[SectionC] : ParentA, ParentB";
    YINI::Lexer lexer3(source3);
    std::vector<YINI::Token> tokens3 = lexer3.scanTokens();
    YINI::Parser parser3(tokens3);
    std::vector<std::unique_ptr<YINI::Stmt>> ast3 = parser3.parse();
    ASSERT_EQ(ast3.size(), 1);
    auto* sectionNode3 = dynamic_cast<YINI::Section*>(ast3[0].get());
    ASSERT_NE(sectionNode3, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode3->name.literal), "SectionC");
    ASSERT_EQ(sectionNode3->parents.size(), 2);
    EXPECT_EQ(std::any_cast<std::string>(sectionNode3->parents[0].literal), "ParentA");
    EXPECT_EQ(std::any_cast<std::string>(sectionNode3->parents[1].literal), "ParentB");
}

TEST(ParserTest, ParsesRegistrationStatement)
{
    std::string source = R"(
        [MySection]
        += 123
        key = "value"
        += "another"
    )";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto* sectionNode = dynamic_cast<YINI::Section*>(ast[0].get());
    ASSERT_NE(sectionNode, nullptr);
    ASSERT_EQ(sectionNode->statements.size(), 3);

    // First statement: += 123
    auto* regNode1 = dynamic_cast<YINI::Register*>(sectionNode->statements[0].get());
    ASSERT_NE(regNode1, nullptr);
    auto* literal1 = dynamic_cast<YINI::Literal*>(regNode1->value.get());
    ASSERT_NE(literal1, nullptr);
    EXPECT_EQ(std::any_cast<double>(literal1->value), 123);

    // Second statement: key = "value"
    auto* kvNode = dynamic_cast<YINI::KeyValue*>(sectionNode->statements[1].get());
    ASSERT_NE(kvNode, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(kvNode->key.literal), "key");

    // Third statement: += "another"
    auto* regNode2 = dynamic_cast<YINI::Register*>(sectionNode->statements[2].get());
    ASSERT_NE(regNode2, nullptr);
    auto* literal2 = dynamic_cast<YINI::Literal*>(regNode2->value.get());
    ASSERT_NE(literal2, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(literal2->value), "another");
}

TEST(ParserTest, ParsesDefineSection)
{
    std::string source = R"(
        [#define]
        my_var = 123
        another = "hello"
    )";
    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.scanTokens();
    YINI::Parser parser(tokens);
    std::vector<std::unique_ptr<YINI::Stmt>> ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);

    auto* defineNode = dynamic_cast<YINI::Define*>(ast[0].get());
    ASSERT_NE(defineNode, nullptr);
    ASSERT_EQ(defineNode->values.size(), 2);

    // my_var = 123
    auto* kv1 = defineNode->values[0].get();
    EXPECT_EQ(std::any_cast<std::string>(kv1->key.literal), "my_var");
    auto* literal1 = dynamic_cast<YINI::Literal*>(kv1->value.get());
    ASSERT_NE(literal1, nullptr);
    EXPECT_EQ(std::any_cast<double>(literal1->value), 123);

    // another = "hello"
    auto* kv2 = defineNode->values[1].get();
    EXPECT_EQ(std::any_cast<std::string>(kv2->key.literal), "another");
    auto* literal2 = dynamic_cast<YINI::Literal*>(kv2->value.get());
    ASSERT_NE(literal2, nullptr);
    EXPECT_EQ(std::any_cast<std::string>(literal2->value), "hello");
}