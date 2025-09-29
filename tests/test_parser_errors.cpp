#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <streambuf>

static std::string read_file_content(const std::string &path)
{
    std::ifstream t(path);
    if (!t.is_open())
        return "";
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return str;
}

TEST(ParserErrorTest, ThrowsOnInvalidSectionName)
{
  const std::string input = "[123_invalid]\nkey = value";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnMissingEqualsInKeyValuePair)
{
  // YINI_TEST_DATA_DIR is a C-string literal defined by CMake.
  const std::string test_data_dir = YINI_TEST_DATA_DIR;
  const std::string input_file_path = test_data_dir + "/invalid_key_value.yini";
  const std::string input = read_file_content(input_file_path);
  ASSERT_FALSE(input.empty()) << "Failed to read test file: " << input_file_path;

  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc, test_data_dir);

  EXPECT_THROW(
      {
        try
        {
          parser.parse();
        }
        catch (const YINI::YiniException &e)
        {
          EXPECT_STREQ("Expected '=' after key 'key_without_equals'.", e.what());
          EXPECT_EQ(e.getLine(), 3); // The error is detected on the next line
          throw;
        }
      },
      YINI::YiniException);
}

TEST(ParserErrorTest, ThrowsOnUnclosedArray)
{
  const std::string input = "[Data]\nmy_array = [1, 2, 3";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  ASSERT_THROW(parser.parse(), YINI::YiniException);
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
        catch (const YINI::YiniException &e)
        {
          EXPECT_STREQ("Expected ':' after map key.", e.what());
          throw;
        }
      },
      YINI::YiniException);

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
        catch (const YINI::YiniException &e)
        {
          EXPECT_STREQ("Expected ',' or '}' in map.", e.what());
          throw;
        }
      },
      YINI::YiniException);

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
        catch (const YINI::YiniException &e)
        {
          // The loop terminates on EOF, and the final check catches the
          // unclosed brace.
          EXPECT_STREQ("Expected '}' to close map.", e.what());
          throw;
        }
      },
      YINI::YiniException);
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