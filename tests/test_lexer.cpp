#include "test_framework.h"
#include "../src/Lexer/Lexer.h"
#include "../src/Lexer/Token.h"

using namespace yini;
using namespace yini_test;

TEST(LexerEmptySource)
{
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 1);  // Only EOF
    ASSERT_TRUE(tokens[0].is(TokenType::EndOfFile));
}

TEST(LexerSimpleInteger)
{
    Lexer lexer("123");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Integer + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Integer));
    ASSERT_EQ(tokens[0].getLexeme(), "123");
}

TEST(LexerSimpleFloat)
{
    Lexer lexer("3.14");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Float + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Float));
    ASSERT_EQ(tokens[0].getLexeme(), "3.14");
}

TEST(LexerString)
{
    Lexer lexer("\"hello world\"");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // String + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::String));
    ASSERT_EQ(tokens[0].getValue(), "hello world");
}

TEST(LexerBoolean)
{
    Lexer lexer("true false");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 3);  // Boolean + Boolean + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Boolean));
    ASSERT_TRUE(tokens[1].is(TokenType::Boolean));
}

TEST(LexerIdentifier)
{
    Lexer lexer("my_var");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Identifier + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Identifier));
    ASSERT_EQ(tokens[0].getLexeme(), "my_var");
}

TEST(LexerKeywords)
{
    Lexer lexer("Dyna Color Coord Path List Array");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 7);  // 6 keywords + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Dyna));
    ASSERT_TRUE(tokens[1].is(TokenType::Color));
    ASSERT_TRUE(tokens[2].is(TokenType::Coord));
    ASSERT_TRUE(tokens[3].is(TokenType::Path));
    ASSERT_TRUE(tokens[4].is(TokenType::List));
    ASSERT_TRUE(tokens[5].is(TokenType::Array));
}

TEST(LexerSection)
{
    Lexer lexer("[Config]");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Section + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Section));
    ASSERT_EQ(tokens[0].getValue(), "Config");
}

TEST(LexerDefineSection)
{
    Lexer lexer("[#define]");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Define + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Define));
}

TEST(LexerIncludeSection)
{
    Lexer lexer("[#include]");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Include + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Include));
}

TEST(LexerSchemaSection)
{
    Lexer lexer("[#schema]");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Schema + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Schema));
}

TEST(LexerOperators)
{
    Lexer lexer("+ - * / % = += :");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 9);  // 8 operators + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Plus));
    ASSERT_TRUE(tokens[1].is(TokenType::Minus));
    ASSERT_TRUE(tokens[2].is(TokenType::Star));
    ASSERT_TRUE(tokens[3].is(TokenType::Slash));
    ASSERT_TRUE(tokens[4].is(TokenType::Percent));
    ASSERT_TRUE(tokens[5].is(TokenType::Assign));
    ASSERT_TRUE(tokens[6].is(TokenType::PlusAssign));
    ASSERT_TRUE(tokens[7].is(TokenType::Colon));
}

TEST(LexerDelimiters)
{
    Lexer lexer("[](){}");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 7);  // 6 delimiters + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::LeftBracket));
    ASSERT_TRUE(tokens[1].is(TokenType::RightBracket));
    ASSERT_TRUE(tokens[2].is(TokenType::LeftParen));
    ASSERT_TRUE(tokens[3].is(TokenType::RightParen));
    ASSERT_TRUE(tokens[4].is(TokenType::LeftBrace));
    ASSERT_TRUE(tokens[5].is(TokenType::RightBrace));
}

TEST(LexerLineComment)
{
    Lexer lexer("// this is a comment\nkey = value");
    auto tokens = lexer.tokenize();
    // Comment, NewLine, Identifier, Assign, Identifier, EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Comment));
    ASSERT_TRUE(tokens[1].is(TokenType::NewLine));
}

TEST(LexerBlockComment)
{
    Lexer lexer("/* multi\nline\ncomment */");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // Comment + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Comment));
}

TEST(LexerMacroRef)
{
    Lexer lexer("@myvar");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // MacroRef + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::MacroRef));
    ASSERT_EQ(tokens[0].getValue(), "myvar");
}

TEST(LexerEnvVar)
{
    Lexer lexer("${WIDTH}");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // EnvVar + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::EnvVar));
    ASSERT_EQ(tokens[0].getValue(), "WIDTH");
}

TEST(LexerCrossRef)
{
    Lexer lexer("@{Config.width}");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 2);  // CrossRef + EOF
    ASSERT_TRUE(tokens[0].is(TokenType::CrossRef));
    ASSERT_EQ(tokens[0].getValue(), "Config.width");
}

TEST(LexerKeyValuePair)
{
    Lexer lexer("key = 123");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 4);  // Identifier, Assign, Integer, EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Identifier));
    ASSERT_EQ(tokens[0].getLexeme(), "key");
    ASSERT_TRUE(tokens[1].is(TokenType::Assign));
    ASSERT_TRUE(tokens[2].is(TokenType::Integer));
    ASSERT_EQ(tokens[2].getLexeme(), "123");
}

TEST(LexerQuickRegister)
{
    Lexer lexer("+= value1");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 3);  // PlusAssign, Identifier, EOF
    ASSERT_TRUE(tokens[0].is(TokenType::PlusAssign));
    ASSERT_TRUE(tokens[1].is(TokenType::Identifier));
}

TEST(LexerComplexExpression)
{
    Lexer lexer("[Config]\nkey = 10 + 20 * 2");
    auto tokens = lexer.tokenize();
    // Section, NewLine, Identifier, Assign, Integer, Plus, Integer, Star, Integer, EOF
    ASSERT_TRUE(tokens[0].is(TokenType::Section));
    ASSERT_TRUE(tokens[1].is(TokenType::NewLine));
    ASSERT_TRUE(tokens[2].is(TokenType::Identifier));
    ASSERT_TRUE(tokens[3].is(TokenType::Assign));
    ASSERT_TRUE(tokens[4].is(TokenType::Integer));
    ASSERT_TRUE(tokens[5].is(TokenType::Plus));
    ASSERT_TRUE(tokens[6].is(TokenType::Integer));
    ASSERT_TRUE(tokens[7].is(TokenType::Star));
    ASSERT_TRUE(tokens[8].is(TokenType::Integer));
}

int main()
{
    return TestRunner::instance().run();
}
