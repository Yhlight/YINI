#include <gtest/gtest.h>
#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"

TEST(ParserErrorTest, ThrowsOnInvalidSectionName)
{
    const std::string input = "[123_invalid]\nkey = value";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnUnclosedArray)
{
    const std::string input = "[Data]\nmy_array = [1, 2, 3";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnUnclosedPair)
{
    const std::string input = "[Data]\nmy_pair = {key: 123";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnMissingColonInPair)
{
    const std::string input = "[Data]\nmy_pair = {key 123}";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnUnclosedMap)
{
    const std::string input = "[Data]\nmy_map = {{key: 1}";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnUndefinedMacro)
{
    const std::string input = "[Data]\nvalue = @undefined_macro";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnInvalidCoordArgs)
{
    const std::string input = "[Data]\npos = Coord(1, \"two\")";
    YINI::YiniDocument doc;
    YINI::Parser parser(input, doc);
    ASSERT_THROW(parser.parse(), YINI::YiniException);
}