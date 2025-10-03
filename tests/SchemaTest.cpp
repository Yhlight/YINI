#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/Validator.h"
#include "Core/YiniException.h"
#include <fstream>
#include <vector>

// Helper to create a file and load it with a YiniManager
static void load_from_source(YINI::YiniManager& manager, const std::string& source, const std::string& filename = "schema_test.yini") {
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();
    manager.load(filename);
}

TEST(SchemaTest, ValidFilePassesValidation) {
    YINI::YiniManager manager;
    std::string source = R"(
        [#schema]
        [Database]
        host = "string, required"
        port = "number, required"
        user = "string, optional"
        [Player]
        name = "string, required"
        inventory = "array[string], optional"
        [#end_schema]

        [Database]
        host = "localhost"
        port = 5432
        [Player]
        name = "Jules"
        inventory = ["sword", "shield"]
    )";
    load_from_source(manager, source);

    const YINI::Schema* schema = manager.get_schema();
    ASSERT_NE(schema, nullptr);

    YINI::Validator validator;
    std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());

    // Expect no errors for a valid file
    ASSERT_TRUE(errors.empty());
}

TEST(SchemaTest, CatchesMissingRequiredKey) {
    YINI::YiniManager manager;
    std::string source = R"(
        [#schema]
        [Database]
        host = "string, required"
        port = "number, required"
        [#end_schema]

        [Database]
        host = "localhost"
    )";
    load_from_source(manager, source);
    const YINI::Schema* schema = manager.get_schema();
    ASSERT_NE(schema, nullptr);
    YINI::Validator validator;
    std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].message, "Required key 'Database.port' is missing.");
}

TEST(SchemaTest, CatchesTypeMismatch) {
    YINI::YiniManager manager;
    std::string source = R"(
        [#schema]
        [Database]
        port = "number, required"
        [#end_schema]

        [Database]
        port = "not-a-number"
    )";
    load_from_source(manager, source);
    const YINI::Schema* schema = manager.get_schema();
    ASSERT_NE(schema, nullptr);
    YINI::Validator validator;
    std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].message, "Type mismatch for 'Database.port': expected number.");
}

TEST(SchemaTest, CatchesArraySubTypeMismatch) {
    YINI::YiniManager manager;
    std::string source = R"(
        [#schema]
        [Player]
        inventory = "array[string], required"
        [#end_schema]

        [Player]
        inventory = ["sword", 123]
    )";
    load_from_source(manager, source);
    const YINI::Schema* schema = manager.get_schema();
    ASSERT_NE(schema, nullptr);
    YINI::Validator validator;
    std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].message, "Type mismatch for 'Player.inventory[]': expected string.");
}

TEST(SchemaTest, CatchesMissingRequiredSection) {
    YINI::YiniManager manager;
    std::string source = R"(
        [#schema]
        [Database]
        host = "string, required"
        [#end_schema]

        [OtherSection]
        key = "value"
    )";
    load_from_source(manager, source);
    const YINI::Schema* schema = manager.get_schema();
    ASSERT_NE(schema, nullptr);
    YINI::Validator validator;
    std::vector<YINI::ValidationError> errors = validator.validate(*schema, manager.get_interpreter());

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].message, "Required section 'Database' is missing.");
}