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
  try {
      parser.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Invalid section name.");
  }
}

TEST(ParserErrorTest, ThrowsOnMissingEqualsInKeyValuePair)
{
  const std::string test_data_dir = YINI_TEST_DATA_DIR;
  const std::string input_file_path = test_data_dir + "/invalid_key_value.yini";
  const std::string input = read_file_content(input_file_path);
  ASSERT_FALSE(input.empty()) << "Failed to read test file: " << input_file_path;

  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc, test_data_dir);

  try {
      parser.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      const auto& err = e.getErrors()[0];
      EXPECT_EQ(err.message, "Expected '=' after key 'key_without_equals'.");
  }
}

TEST(ParserErrorTest, ThrowsOnUnclosedArray)
{
  const std::string input = "[Data]\nmy_array = [1, 2, 3";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  try {
      parser.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Expected ']' to close array.");
  }
}

TEST(ParserErrorTest, ThrowsOnMalformedMap)
{
  const std::string input1 = "[Data]\nmy_map = { key \"value\" }";
  YINI::YiniDocument doc1;
  YINI::Parser parser1(input1, doc1);
  try {
      parser1.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Expected ':' after map key.");
  }

  const std::string input2 = "[Data]\nmy_map = { key1: 1 key2: 2 }";
  YINI::YiniDocument doc2;
  YINI::Parser parser2(input2, doc2);
  try {
      parser2.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Expected ',' or '}' in map.");
  }

  const std::string input3 = "[Data]\nmy_map = { key: 1,";
  YINI::YiniDocument doc3;
  YINI::Parser parser3(input3, doc3);
  try {
      parser3.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Expected '}' to close map.");
  }
}

TEST(ParserErrorTest, ThrowsOnUndefinedMacro)
{
  const std::string input = "[Data]\nvalue = @undefined_macro";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  try {
      parser.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Undefined macro: undefined_macro");
  }
}

TEST(ParserErrorTest, ThrowsOnInvalidCoordArgs)
{
  const std::string input = "[Data]\npos = Coord(1, \"two\")";
  YINI::YiniDocument doc;
  YINI::Parser parser(input, doc);
  try {
      parser.parse();
      FAIL() << "Expected YiniParsingException";
  } catch (const YINI::YiniParsingException& e) {
      ASSERT_EQ(e.getErrors().size(), 1);
      EXPECT_EQ(e.getErrors()[0].message, "Coord parameters must be numeric.");
  }
}