#include <gtest/gtest.h>
#include "Parser/parser.h"
#include "Lexer/lexer.h"

TEST(ParserTest, ParseSectionAndKeyValuePair) {
    std::string input = "[Config]\nkey = \"value\"";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Config"));
    EXPECT_EQ(std::get<std::string>(config["Config"]["key"]), "value");
}

TEST(ParserTest, ParseInteger) {
    std::string input = "[Numbers]\nint_val = 123";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_EQ(std::get<int>(config["Numbers"]["int_val"]), 123);
}

TEST(ParserTest, ParseFloat) {
    std::string input = "[Numbers]\nfloat_val = 3.14";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_DOUBLE_EQ(std::get<double>(config["Numbers"]["float_val"]), 3.14);
}

TEST(ParserTest, ParseBoolean) {
    std::string input = "[Booleans]\nbool_true = true\nbool_false = false";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Booleans"));
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_true"]), true);
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_false"]), false);
}