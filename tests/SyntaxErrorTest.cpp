#include <gtest/gtest.h>

#include <fstream>

#include "Core/YiniException.h"
#include "Core/YiniManager.h"

// Helper function to test for ParsingError during the loading process.
void expect_parsing_error(const std::string& source, const std::string& test_name, int expected_line, int expected_col,
                          const std::string& expected_message) {
    const std::string filename = "syntax_error_" + test_name + ".yini";
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();

    YINI::YiniManager manager;
    try {
        manager.load(filename);
        FAIL() << "Expected ParsingError for source:\n" << source;
    } catch (const YINI::ParsingError& e) {
        EXPECT_EQ(e.line(), expected_line) << "Incorrect line number for test: " << test_name;
        EXPECT_EQ(e.column(), expected_col) << "Incorrect column number for test: " << test_name;
        EXPECT_EQ(e.filepath(), filename) << "Incorrect filepath for test: " << test_name;
        EXPECT_EQ(std::string(e.what()), expected_message) << "Incorrect message for test: " << test_name;
    } catch (const std::exception& e) {
        FAIL() << "Expected ParsingError but got std::exception: " << e.what();
    } catch (...) {
        FAIL() << "Expected ParsingError but got an unknown exception.";
    }
}

TEST(SyntaxErrorTest, ThrowsOnUnclosedSection) {
    expect_parsing_error("[Section\nkey=val", "unclosed_section", 2, 1, "Expect ']' after section name. But got 'key' instead.");
}

TEST(SyntaxErrorTest, ThrowsOnUnterminatedString) {
    expect_parsing_error("[Test]\nkey = \"hello", "unterminated_string", 2, 13, "Unterminated string.");
}

TEST(SyntaxErrorTest, ThrowsOnUnterminatedBlockComment) {
    expect_parsing_error("/* comment", "unterminated_comment", 1, 10, "Unterminated block comment.");
}

TEST(SyntaxErrorTest, ThrowsOnUnexpectedCharacter) {
    expect_parsing_error("^", "unexpected_char", 1, 1, "Unexpected character.");
}

TEST(SyntaxErrorTest, ThrowsOnMissingValueAfterEquals) {
    expect_parsing_error("[Section]\nkey = ", "missing_value", 2, 7, "Expect expression.");
}

TEST(SyntaxErrorTest, ThrowsOnInvalidExpression) {
    // A closing bracket is not a valid start of an expression.
    expect_parsing_error("[Section]\nkey = ]", "invalid_expr", 2, 7, "Expect expression.");
}

TEST(SyntaxErrorTest, ThrowsOnUnclosedArray) {
    expect_parsing_error("[Test]\nkey = [1, 2", "unclosed_array", 2, 12, "Expect ']' after array elements. But got '' instead.");
}

TEST(SyntaxErrorTest, ThrowsOnUnclosedMap) {
    expect_parsing_error("[Test]\nkey = {\"a\": 1", "unclosed_map", 2, 14, "Expect '}' after map pairs. But got '' instead.");
}

TEST(SyntaxErrorTest, ThrowsOnMissingMapColon) {
    expect_parsing_error("[Test]\nkey = {\"a\" 1}", "missing_colon", 2, 12, "Expect ':' after map key. But got '1' instead.");
}