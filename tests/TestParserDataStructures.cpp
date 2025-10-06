#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

TEST(ParserDataStructuresTest, MapAndTupleParsing) {
    std::string input = R"(
[Data]
tuple = {"key": "value"}
map = {"key1": 1, "key2": true}
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->pairs.size(), 2);

    // Test the tuple (single-pair map)
    auto tuple_pair = section->pairs[0];
    ASSERT_EQ(tuple_pair->key->value, "tuple");
    auto tuple_lit = std::dynamic_pointer_cast<YINI::MapLiteral>(tuple_pair->value);
    ASSERT_NE(tuple_lit, nullptr);
    ASSERT_EQ(tuple_lit->pairs.size(), 1);

    auto tuple_it = tuple_lit->pairs.begin();
    auto tuple_key = std::dynamic_pointer_cast<YINI::StringLiteral>(tuple_it->first);
    auto tuple_val = std::dynamic_pointer_cast<YINI::StringLiteral>(tuple_it->second);
    ASSERT_NE(tuple_key, nullptr);
    ASSERT_EQ(tuple_key->value, "\"key\"");
    ASSERT_NE(tuple_val, nullptr);
    ASSERT_EQ(tuple_val->value, "\"value\"");


    // Test the multi-pair map
    auto map_pair = section->pairs[1];
    ASSERT_EQ(map_pair->key->value, "map");
    auto map_lit = std::dynamic_pointer_cast<YINI::MapLiteral>(map_pair->value);
    ASSERT_NE(map_lit, nullptr);
    ASSERT_EQ(map_lit->pairs.size(), 2);

    // Note: A map of shared_ptrs requires a custom comparator for `find` to work correctly.
    // For this test, we will iterate through the pairs to find the keys.
    bool found_key1 = false;
    bool found_key2 = false;
    for(const auto& p : map_lit->pairs) {
        auto key = std::dynamic_pointer_cast<YINI::StringLiteral>(p.first);
        if (key && key->value == "\"key1\"") {
            found_key1 = true;
            auto val = std::dynamic_pointer_cast<YINI::IntegerLiteral>(p.second);
            ASSERT_NE(val, nullptr);
            ASSERT_EQ(val->value, 1);
        } else if (key && key->value == "\"key2\"") {
            found_key2 = true;
            auto val = std::dynamic_pointer_cast<YINI::BooleanLiteral>(p.second);
            ASSERT_NE(val, nullptr);
            ASSERT_EQ(val->value, true);
        }
    }
    ASSERT_TRUE(found_key1);
    ASSERT_TRUE(found_key2);
}

TEST(ParserDataStructuresTest, CallExpressionParsing) {
    std::string input = R"(
[Visuals]
background_color = Color(255, 192, 203)
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);

    auto pair = section->pairs[0];
    auto call_expr = std::dynamic_pointer_cast<YINI::CallExpression>(pair->value);
    ASSERT_NE(call_expr, nullptr);

    auto func_ident = std::dynamic_pointer_cast<YINI::Identifier>(call_expr->function);
    ASSERT_NE(func_ident, nullptr);
    ASSERT_EQ(func_ident->value, "Color");

    ASSERT_EQ(call_expr->arguments.size(), 3);
    auto arg1 = std::dynamic_pointer_cast<YINI::IntegerLiteral>(call_expr->arguments[0]);
    ASSERT_NE(arg1, nullptr);
    ASSERT_EQ(arg1->value, 255);

    auto arg2 = std::dynamic_pointer_cast<YINI::IntegerLiteral>(call_expr->arguments[1]);
    ASSERT_NE(arg2, nullptr);
    ASSERT_EQ(arg2->value, 192);

    auto arg3 = std::dynamic_pointer_cast<YINI::IntegerLiteral>(call_expr->arguments[2]);
    ASSERT_NE(arg3, nullptr);
    ASSERT_EQ(arg3->value, 203);
}

TEST(ParserDataStructuresTest, GroupedAndCollectionParsing) {
    // Test a grouped expression
    std::string input1 = R"(
[Data]
val = (1 + 2) * 3
)";
    YINI::Lexer lexer1(input1);
    YINI::Parser parser1(lexer1);
    auto program1 = parser1.ParseProgram();
    ASSERT_NE(program1, nullptr);
    auto section1 = std::dynamic_pointer_cast<YINI::Section>(program1->statements[0]);
    ASSERT_NE(section1, nullptr);
    auto pair1 = section1->pairs[0];
    auto infix1 = std::dynamic_pointer_cast<YINI::InfixExpression>(pair1->value);
    ASSERT_NE(infix1, nullptr);
    ASSERT_EQ(infix1->op, "*");

    // Test a multi-element collection
    std::string input2 = R"(
[Data]
val = (1, "two")
)";
    YINI::Lexer lexer2(input2);
    YINI::Parser parser2(lexer2);
    auto program2 = parser2.ParseProgram();
    ASSERT_NE(program2, nullptr);
    auto section2 = std::dynamic_pointer_cast<YINI::Section>(program2->statements[0]);
    ASSERT_NE(section2, nullptr);
    auto pair2 = section2->pairs[0];
    auto coll2 = std::dynamic_pointer_cast<YINI::CollectionLiteral>(pair2->value);
    ASSERT_NE(coll2, nullptr);
    ASSERT_EQ(coll2->elements.size(), 2);

    // Test a single-element collection
    std::string input3 = R"(
[Data]
val = (1,)
)";
    YINI::Lexer lexer3(input3);
    YINI::Parser parser3(lexer3);
    auto program3 = parser3.ParseProgram();
    ASSERT_NE(program3, nullptr);
    auto section3 = std::dynamic_pointer_cast<YINI::Section>(program3->statements[0]);
    ASSERT_NE(section3, nullptr);
    auto pair3 = section3->pairs[0];
    auto coll3 = std::dynamic_pointer_cast<YINI::CollectionLiteral>(pair3->value);
    ASSERT_NE(coll3, nullptr);
    ASSERT_EQ(coll3->elements.size(), 1);
}