#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <gtest/gtest.h>

TEST(ParserErrorTest, ThrowsOnInvalidSectionName)
{
  const std::string input = "[123_invalid]\nkey = value";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::ParsingException);
}

TEST(ParserErrorTest, ThrowsOnMissingIncludeFile)
{
  const std::string input = "[#include]\n+= non_existent_file.yini";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::IOException);
}

TEST(LogicErrorTest, ThrowsOnCircularInheritance)
{
  const std::string input = "[A:B]\n[B:C]\n[C:A]";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  parser.parse();
  ASSERT_THROW(doc.resolveInheritance(), YINI::LogicException);
}

TEST(ParserErrorTest, ThrowsOnUnclosedArray)
{
  const std::string input = "[Data]\nmy_array = [1, 2, 3";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::ParsingException);
}

TEST(ParserErrorTest, ThrowsOnMalformedMap)
{
  // Test case 1: Missing colon
  const std::string input1 = "[Data]\nmy_map = { key \"value\" }";
  YINI::YiniDocument doc1;
  YINI::Parser parser1(input1, doc1);
  EXPECT_THROW(
      {
        try
        {
          parser1.parse();
        }
        catch (const YINI::ParsingException &e)
        {
          EXPECT_STREQ("Expected ':' after map key.", e.what());
          throw;
        }
      },
      YINI::ParsingException);

  // Test case 2: Missing comma
  const std::string input2 = "[Data]\nmy_map = { key1: 1 key2: 2 }";
  YINI::YiniDocument doc2;
  YINI::Parser parser2(input2, doc2);
  EXPECT_THROW(
      {
        try
        {
          parser2.parse();
        }
        catch (const YINI::ParsingException &e)
        {
          EXPECT_STREQ("Expected ',' or '}' in object.", e.what());
          throw;
        }
      },
      YINI::ParsingException);

  // Test case 3: Unclosed map
  const std::string input3 = "[Data]\nmy_map = { key: 1,";
  YINI::YiniDocument doc3;
  YINI::Parser parser3(input3, doc3);
  EXPECT_THROW(
      {
        try
        {
          parser3.parse();
        }
        catch (const YINI::ParsingException &e)
        {
          // The loop terminates on EOF, and the final check catches the
          // unclosed brace.
          EXPECT_STREQ("Expected '}' to close object.", e.what());
          throw;
        }
      },
      YINI::ParsingException);
}

TEST(ParserErrorTest, ThrowsOnUndefinedMacro)
{
  const std::string input = "[Data]\nvalue = @undefined_macro";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::ParsingException);
}

TEST(ParserErrorTest, ThrowsOnInvalidCoordArgs)
{
  const std::string input = "[Data]\npos = Coord(1, \"two\")";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::ParsingException);
}