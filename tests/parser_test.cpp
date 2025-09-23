#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include <vector>

using namespace Yini;

TEST(ParserTest, TestSectionWithInheritance) {
    std::string input = "[Child] : Parent1, Parent2";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();

    ASSERT_TRUE(parser.getErrors().empty());
    ASSERT_EQ(doc->statements.size(), 1);

    auto* section = dynamic_cast<Ast::Section*>(doc->statements[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->name->value, "Child");
    ASSERT_EQ(section->parents.size(), 2);
    ASSERT_EQ(section->parents[0]->value, "Parent1");
    ASSERT_EQ(section->parents[1]->value, "Parent2");
}

TEST(ParserTest, TestMacroReference) {
    std::string input = "key = @macro_name";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();

    ASSERT_TRUE(parser.getErrors().empty());
    auto* kvp = dynamic_cast<Ast::KeyValuePair*>(doc->statements[0].get());
    ASSERT_NE(kvp, nullptr);

    auto* macro_ref = dynamic_cast<Ast::MacroReference*>(kvp->value.get());
    ASSERT_NE(macro_ref, nullptr);
    ASSERT_EQ(macro_ref->name->value, "macro_name");
}

TEST(ParserTest, TestArrayLiteral) {
    std::string input = "key = [1, \"two\", 3.0]";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();

    ASSERT_TRUE(parser.getErrors().empty());
    auto* kvp = dynamic_cast<Ast::KeyValuePair*>(doc->statements[0].get());
    auto* arr = dynamic_cast<Ast::ArrayLiteral*>(kvp->value.get());
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(arr->elements.size(), 3);
}

TEST(ParserTest, TestFunctionCall) {
    std::string input = "key = MyFunc(arg1, 100)";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();

    ASSERT_TRUE(parser.getErrors().empty());
    auto* kvp = dynamic_cast<Ast::KeyValuePair*>(doc->statements[0].get());
    auto* call = dynamic_cast<Ast::FunctionCall*>(kvp->value.get());
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->functionName->value, "MyFunc");
    ASSERT_EQ(call->arguments.size(), 2);
}

TEST(ParserTest, TestMapLiteral) {
    std::string input = "key = {{k1: 1}, {k2: 2}}";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();

    ASSERT_TRUE(parser.getErrors().empty());
    auto* kvp = dynamic_cast<Ast::KeyValuePair*>(doc->statements[0].get());
    auto* map = dynamic_cast<Ast::MapLiteral*>(kvp->value.get());
    ASSERT_NE(map, nullptr);
    ASSERT_EQ(map->elements.size(), 2);

    auto* kvp1 = map->elements[0].get();
    ASSERT_EQ(kvp1->key->value, "k1");
    auto* int1 = dynamic_cast<Ast::IntegerLiteral*>(kvp1->value.get());
    ASSERT_EQ(int1->value, 1);
}
