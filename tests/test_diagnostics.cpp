#include <gtest/gtest.h>
#include "Parser/parser.h"

TEST(DiagnosticsTest, ThrowsParsingExceptionWithCorrectLocation) {
    std::string invalid_input = R"(
[Section1]
key1 = "value1"

[Section2]
key2 = = invalid_value
)";

    Parser parser;
    try {
        parser.parse(invalid_input);
        FAIL() << "Expected a ParsingException to be thrown.";
    } catch (const ParsingException& e) {
        EXPECT_EQ(e.getLine(), 6);
        EXPECT_EQ(e.getColumn(), 8); // The position of the second '='
        EXPECT_STREQ(e.what(), "Unexpected value token: =");
    } catch (...) {
        FAIL() << "Expected a ParsingException, but a different exception was thrown.";
    }
}