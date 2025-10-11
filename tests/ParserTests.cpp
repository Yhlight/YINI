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

    try {
        parser.parse();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("Error at line 1"), std::string::npos);
    }
}

TEST(ParserTests, ParsesBooleanLiterals)
{
    std::string source = "[Booleans]\ntrue_val = true\nfalse_val = false";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    auto true_kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(true_kv, nullptr);
    EXPECT_EQ(true_kv->key.lexeme, "true_val");
    auto true_expr = dynamic_cast<YINI::AST::BoolExpr*>(true_kv->value.get());
    ASSERT_NE(true_expr, nullptr);
    EXPECT_EQ(true_expr->value, true);

    auto false_kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[1].get());
    ASSERT_NE(false_kv, nullptr);
    EXPECT_EQ(false_kv->key.lexeme, "false_val");
    auto false_expr = dynamic_cast<YINI::AST::BoolExpr*>(false_kv->value.get());
    ASSERT_NE(false_expr, nullptr);
    EXPECT_EQ(false_expr->value, false);
}

TEST(ParserTests, ParsesArrayOfNumbers)
{
    std::string source = "[Arrays]\nnumbers = [1, 2, 3]";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto array_expr = dynamic_cast<YINI::AST::ArrayExpr*>(keyValue->value.get());
    ASSERT_NE(array_expr, nullptr);
    ASSERT_EQ(array_expr->elements.size(), 3);

    auto e1 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[0].get());
    ASSERT_NE(e1, nullptr);
    EXPECT_EQ(std::get<double>(e1->value.literal), 1);

    auto e2 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[1].get());
    ASSERT_NE(e2, nullptr);
    EXPECT_EQ(std::get<double>(e2->value.literal), 2);

    auto e3 = dynamic_cast<YINI::AST::LiteralExpr*>(array_expr->elements[2].get());
    ASSERT_NE(e3, nullptr);
    EXPECT_EQ(std::get<double>(e3->value.literal), 3);
}

TEST(ParserTests, ParsesEmptyArray)
{
    std::string source = "[Arrays]\nempty = []";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto array_expr = dynamic_cast<YINI::AST::ArrayExpr*>(keyValue->value.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements.size(), 0);
}

TEST(ParserTests, ParsesSectionWithSingleInheritance)
{
    std::string source = "[Child] : Parent";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Child");
    ASSERT_EQ(section->parent_sections.size(), 1);
    EXPECT_EQ(section->parent_sections[0].lexeme, "Parent");
}

TEST(ParserTests, ParsesSectionWithMultipleInheritance)
{
    std::string source = "[Child] : Parent1, Parent2, Parent3";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->name.lexeme, "Child");
    ASSERT_EQ(section->parent_sections.size(), 3);
    EXPECT_EQ(section->parent_sections[0].lexeme, "Parent1");
    EXPECT_EQ(section->parent_sections[1].lexeme, "Parent2");
    EXPECT_EQ(section->parent_sections[2].lexeme, "Parent3");
}

TEST(ParserTests, ParsesDefineSection)
{
    std::string source = "[#define]\nname = \"YINI\"\nversion = 1";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto define_section = dynamic_cast<YINI::AST::DefineSectionStmt*>(ast[0].get());
    ASSERT_NE(define_section, nullptr);
    ASSERT_EQ(define_section->definitions.size(), 2);
    EXPECT_EQ(define_section->definitions[0]->key.lexeme, "name");
    EXPECT_EQ(define_section->definitions[1]->key.lexeme, "version");
}

TEST(ParserTests, ParsesMacroReference)
{
    std::string source = "[MyConfig]\nvalue = @some_macro";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto macro_expr = dynamic_cast<YINI::AST::MacroExpr*>(keyValue->value.get());
    ASSERT_NE(macro_expr, nullptr);
    EXPECT_EQ(macro_expr->name.lexeme, "some_macro");
}

TEST(ParserTests, ParsesDynaExpression)
{
    std::string source = "[MyConfig]\nvalue = Dyna(123)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto dyna_expr = dynamic_cast<YINI::AST::DynaExpr*>(keyValue->value.get());
    ASSERT_NE(dyna_expr, nullptr);
    auto literal_expr = dynamic_cast<YINI::AST::LiteralExpr*>(dyna_expr->expression.get());
    ASSERT_NE(literal_expr, nullptr);
    EXPECT_EQ(std::get<double>(literal_expr->value.literal), 123);
}

TEST(ParserTests, ParsesPathExpression)
{
    std::string source = "[MyConfig]\nmy_path = path(\"/usr/local/bin\")";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto path_expr = dynamic_cast<YINI::AST::PathExpr*>(keyValue->value.get());
    ASSERT_NE(path_expr, nullptr);
    EXPECT_EQ(path_expr->path, "/usr/local/bin");
}

TEST(ParserTests, ParsesListExpression)
{
    std::string source = "[MyConfig]\nmy_list = list(1, \"two\")";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(keyValue, nullptr);

    auto list_expr = dynamic_cast<YINI::AST::ListExpr*>(keyValue->value.get());
    ASSERT_NE(list_expr, nullptr);
    ASSERT_EQ(list_expr->elements.size(), 2);
}

TEST(ParserTests, ParsesQuickRegistration)
{
    std::string source = "[MyReg]\n+= 1\n+= \"two\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    auto reg1 = dynamic_cast<YINI::AST::QuickRegStmt*>(section->statements[0].get());
    ASSERT_NE(reg1, nullptr);
    auto reg2 = dynamic_cast<YINI::AST::QuickRegStmt*>(section->statements[1].get());
    ASSERT_NE(reg2, nullptr);
}

TEST(ParserTests, ParsesArrayFuncSyntax)
{
    std::string source = "[MySection]\nmy_array = array(1, \"two\")";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto* section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 1);
    auto* array_stmt = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(array_stmt, nullptr);
    ASSERT_NE(dynamic_cast<YINI::AST::ArrayExpr*>(array_stmt->value.get()), nullptr);
}

TEST(ParserTests, Parses2DArray)
{
    std::string source = "[MySection]\nmy_array = [[1, 2], [3, 4]]";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto* section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    auto* kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv, nullptr);
    auto* array_expr = dynamic_cast<YINI::AST::ArrayExpr*>(kv->value.get());
    ASSERT_NE(array_expr, nullptr);
    ASSERT_EQ(array_expr->elements.size(), 2);

    auto* sub_array1 = dynamic_cast<YINI::AST::ArrayExpr*>(array_expr->elements[0].get());
    ASSERT_NE(sub_array1, nullptr);
    ASSERT_EQ(sub_array1->elements.size(), 2);

    auto* sub_array2 = dynamic_cast<YINI::AST::ArrayExpr*>(array_expr->elements[1].get());
    ASSERT_NE(sub_array2, nullptr);
    ASSERT_EQ(sub_array2->elements.size(), 2);
}