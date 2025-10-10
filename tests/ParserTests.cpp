#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/AST.h"

TEST(ParserTests, ParsesEmptyInput)
{
    std::string source = "";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 0);
}

TEST(ParserTests, ParsesTopLevelKeyValue)
{
    std::string source = "key = \"value\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto keyValue = dynamic_cast<YINI::AST::KeyValueStmt*>(ast[0].get());
    ASSERT_NE(keyValue, nullptr);
    EXPECT_EQ(keyValue->key.lexeme, "key");
}

TEST(ParserTests, ParsesMultipleSections)
{
    std::string source = "[Section1]\nkey1 = 1\n[Section2]\nkey2 = 2";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 2);
    auto section1 = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section1, nullptr);
    EXPECT_EQ(section1->name.lexeme, "Section1");
    auto section2 = dynamic_cast<YINI::AST::SectionStmt*>(ast[1].get());
    ASSERT_NE(section2, nullptr);
    EXPECT_EQ(section2->name.lexeme, "Section2");
}

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

TEST(ParserTests, ParsesFloatLiteral)
{
    std::string source = "[Floats]\nvalue = 3.14";
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
    EXPECT_EQ(std::get<double>(literal->value.literal), 3.14);
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

TEST(ParserTests, ParsesSet)
{
    std::string source = "[Sets]\nmy_set = (1, \"two\", true)\nsingle_element_set = (\"one\",)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    auto kv1 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv1, nullptr);
    auto set_expr = dynamic_cast<YINI::AST::SetExpr*>(kv1->value.get());
    ASSERT_NE(set_expr, nullptr);
    ASSERT_EQ(set_expr->elements.size(), 3);

    auto kv2 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[1].get());
    ASSERT_NE(kv2, nullptr);
    auto single_element_set_expr = dynamic_cast<YINI::AST::SetExpr*>(kv2->value.get());
    ASSERT_NE(single_element_set_expr, nullptr);
    ASSERT_EQ(single_element_set_expr->elements.size(), 1);
}

TEST(ParserTests, ParsesMap)
{
    std::string source = "[Maps]\nmy_map = {key1: 1, key2: \"value2\"}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 1);

    auto kv = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv, nullptr);
    auto map_expr = dynamic_cast<YINI::AST::MapExpr*>(kv->value.get());
    ASSERT_NE(map_expr, nullptr);
    ASSERT_EQ(map_expr->elements.size(), 2);
    EXPECT_EQ(map_expr->elements[0].first.lexeme, "key1");
    EXPECT_EQ(map_expr->elements[1].first.lexeme, "key2");
}

TEST(ParserTests, ParsesColor)
{
    std::string source = "[Colors]\nhex_color = #FF0000\nrgb_color = color(0, 255, 0)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    // Test hex color
    auto kv1 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv1, nullptr);
    auto hex_color_expr = dynamic_cast<YINI::AST::ColorExpr*>(kv1->value.get());
    ASSERT_NE(hex_color_expr, nullptr);
    EXPECT_EQ(hex_color_expr->r, 255);
    EXPECT_EQ(hex_color_expr->g, 0);
    EXPECT_EQ(hex_color_expr->b, 0);

    // Test rgb color
    auto kv2 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[1].get());
    ASSERT_NE(kv2, nullptr);
    auto rgb_color_expr = dynamic_cast<YINI::AST::ColorExpr*>(kv2->value.get());
    ASSERT_NE(rgb_color_expr, nullptr);
    EXPECT_EQ(rgb_color_expr->r, 0);
    EXPECT_EQ(rgb_color_expr->g, 255);
    EXPECT_EQ(rgb_color_expr->b, 0);
}

TEST(ParserTests, ParsesCoord)
{
    std::string source = "[Coordinates]\ncoord2d = coord(10, 20)\ncoord3d = coord(1, 2, 3.5)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_EQ(ast.size(), 1);
    auto section = dynamic_cast<YINI::AST::SectionStmt*>(ast[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements.size(), 2);

    // Test 2D coordinate
    auto kv1 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[0].get());
    ASSERT_NE(kv1, nullptr);
    auto coord2d_expr = dynamic_cast<YINI::AST::CoordExpr*>(kv1->value.get());
    ASSERT_NE(coord2d_expr, nullptr);
    auto x2d = dynamic_cast<YINI::AST::LiteralExpr*>(coord2d_expr->x.get());
    ASSERT_NE(x2d, nullptr);
    EXPECT_EQ(std::get<double>(x2d->value.literal), 10);
    auto y2d = dynamic_cast<YINI::AST::LiteralExpr*>(coord2d_expr->y.get());
    ASSERT_NE(y2d, nullptr);
    EXPECT_EQ(std::get<double>(y2d->value.literal), 20);
    EXPECT_EQ(coord2d_expr->z, nullptr);


    // Test 3D coordinate
    auto kv2 = dynamic_cast<YINI::AST::KeyValueStmt*>(section->statements[1].get());
    ASSERT_NE(kv2, nullptr);
    auto coord3d_expr = dynamic_cast<YINI::AST::CoordExpr*>(kv2->value.get());
    ASSERT_NE(coord3d_expr, nullptr);
    auto x3d = dynamic_cast<YINI::AST::LiteralExpr*>(coord3d_expr->x.get());
    ASSERT_NE(x3d, nullptr);
    EXPECT_EQ(std::get<double>(x3d->value.literal), 1);
    auto y3d = dynamic_cast<YINI::AST::LiteralExpr*>(coord3d_expr->y.get());
    ASSERT_NE(y3d, nullptr);
    EXPECT_EQ(std::get<double>(y3d->value.literal), 2);
    auto z3d = dynamic_cast<YINI::AST::LiteralExpr*>(coord3d_expr->z.get());
    ASSERT_NE(z3d, nullptr);
    EXPECT_EQ(std::get<double>(z3d->value.literal), 3.5);
}

TEST(ParserTests, ParsesExplicitArray)
{
    std::string source = "[Arrays]\nmy_array = array(1, \"two\")";
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
    ASSERT_EQ(array_expr->elements.size(), 2);
}

TEST(ParserTests, ParsesSectionWithSingleInheritance)
{
    std::string source = "[Child : Parent]";
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
    std::string source = "[Child : Parent1, Parent2,Parent3]";
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