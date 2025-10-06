#include "gtest/gtest.h"
#include "YINI/Parser.h"
#include "YINI/Lexer.h"
#include "YINI/Ast.h"

TEST(ParserInheritanceTest, SingleParent) {
    std::string input = "[Child : Parent]";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->name, "Child");

    ASSERT_EQ(section->parents.size(), 1);
    ASSERT_EQ(section->parents[0]->value, "Parent");
}

TEST(ParserInheritanceTest, MultipleParents) {
    std::string input = "[GrandChild : Child, Parent]";
    YINI::Lexer lexer(input);
    YINI::Parser parser(lexer);

    auto program = parser.ParseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto section = std::dynamic_pointer_cast<YINI::Section>(program->statements[0]);
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->name, "GrandChild");

    ASSERT_EQ(section->parents.size(), 2);
    ASSERT_EQ(section->parents[0]->value, "Child");
    ASSERT_EQ(section->parents[1]->value, "Parent");
}