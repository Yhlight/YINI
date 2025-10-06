#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

TEST(ParserTest, ParseSectionWithIntegerKeyValuePair) {
    std::string input = R"(
[TestSection]
key1 = 123
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto stmt = program->statements[0];
    ASSERT_NE(stmt, nullptr);
    auto section = std::dynamic_pointer_cast<YINI::Section>(stmt);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->name, "TestSection");

    ASSERT_EQ(section->pairs.size(), 1);
    auto pair = section->pairs[0];
    ASSERT_NE(pair, nullptr);

    // Test the key
    ASSERT_NE(pair->key, nullptr);
    ASSERT_EQ(pair->key->value, "key1");

    // Test the value
    ASSERT_NE(pair->value, nullptr);
    auto int_literal = std::dynamic_pointer_cast<YINI::IntegerLiteral>(pair->value);
    ASSERT_NE(int_literal, nullptr);
    ASSERT_EQ(int_literal->value, 123);
}