#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"

TEST(ParserBugTests, CanParseMultipleComplexValuesInOneSection)
{
    std::string source = R"([ComplexSection]
color = #FFC0CB
coord = coord(1.5, 2.5)
map = { key1: "value1", key2: 100 }
nestedArray = [[1, 2], [3, 4]]
)";

    try
    {
        YINI::Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        YINI::Parser parser(tokens);
        auto ast = parser.parse();
        SUCCEED();
    }
    catch (const std::runtime_error& e)
    {
        FAIL() << "Parser failed with exception: " << e.what();
    }
}

TEST(ParserBugTests, ThrowsErrorOnTopLevelKeyValuePair) {
    std::string source = "top_level_key = 123\n[Section]\nkey = 456";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    ASSERT_THROW(parser.parse(), std::runtime_error);
}
