#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

TEST(ParserMacrosTest, DefineAndReferenceMacro) {
    std::string input = R"(
[#define]
primary_color = "blue"

[Colors]
background = @primary_color
)";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 2);

    // Test the [#define] block
    auto define_stmt = std::dynamic_pointer_cast<YINI::DefineStatement>(program->statements[0]);
    ASSERT_NE(define_stmt, nullptr);
    ASSERT_EQ(define_stmt->pairs.size(), 1);
    auto define_pair = define_stmt->pairs[0];
    ASSERT_EQ(define_pair->key->value, "primary_color");
    auto define_val = std::dynamic_pointer_cast<YINI::StringLiteral>(define_pair->value);
    ASSERT_NE(define_val, nullptr);
    ASSERT_EQ(define_val->value, "\"blue\"");

    // Test the section with the macro reference
    auto section_stmt = std::dynamic_pointer_cast<YINI::Section>(program->statements[1]);
    ASSERT_NE(section_stmt, nullptr);
    ASSERT_EQ(section_stmt->pairs.size(), 1);
    auto ref_pair = section_stmt->pairs[0];
    ASSERT_EQ(ref_pair->key->value, "background");
    auto macro_ref = std::dynamic_pointer_cast<YINI::MacroReference>(ref_pair->value);
    ASSERT_NE(macro_ref, nullptr);
    ASSERT_EQ(macro_ref->name, "primary_color");
}