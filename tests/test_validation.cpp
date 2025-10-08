#include <gtest/gtest.h>
#include "Parser/parser.h"

class SchemaValidationTest : public ::testing::Test {
protected:
    Parser schema_parser;

    void SetUp() override {
        // Load the schema once for all tests in this suite
        schema_parser.parseFile("tests/schema.yini");
    }
};

TEST_F(SchemaValidationTest, ValidConfig) {
    Parser config_parser;
    auto config = config_parser.parseFile("tests/config_valid.yini");

    // This should not throw an exception
    EXPECT_NO_THROW(schema_parser.validate(config));
}

TEST_F(SchemaValidationTest, MissingRequiredKeyError) {
    Parser config_parser;
    auto config = config_parser.parseFile("tests/config_invalid_missing.yini");

    // The 'isOld' key is required and its empty behavior is 'e' (error)
    // We need to modify the config to test this specific case
    config["Visual"].erase("isOld");

    EXPECT_THROW(schema_parser.validate(config), std::runtime_error);
}

TEST_F(SchemaValidationTest, MissingRequiredKeyDefault) {
    Parser config_parser;
    auto config = config_parser.parseFile("tests/config_invalid_missing.yini");

    // The 'width' key is required and has a default value
    schema_parser.validate(config);

    ASSERT_TRUE(config.count("Visual"));
    ASSERT_TRUE(config["Visual"].count("width"));
    EXPECT_EQ(std::get<int>(config["Visual"]["width"]), 1280);
}

TEST_F(SchemaValidationTest, InvalidType) {
    Parser config_parser;
    auto config = config_parser.parseFile("tests/config_invalid_type.yini");

    EXPECT_THROW(schema_parser.validate(config), std::runtime_error);
}

TEST_F(SchemaValidationTest, OutOfRange) {
    Parser config_parser;
    auto config = config_parser.parseFile("tests/config_invalid_range.yini");

    EXPECT_THROW(schema_parser.validate(config), std::runtime_error);
}