#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

TEST(ParserTest, ParseSectionAndKeyValuePair) {
    std::string input = R"(
[TestSection]
key1 = "value1"
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto stmt = program->statements[0];
    auto section = std::dynamic_pointer_cast<YINI::Section>(stmt);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->name, "TestSection");

    ASSERT_EQ(section->pairs.size(), 1);
    auto pair = section->pairs[0];
    ASSERT_NE(pair, nullptr);
    ASSERT_EQ(pair->key, "key1");
    ASSERT_EQ(pair->value, "\"value1\""); // For now, we'll just check the raw string value
}