#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>
#include <string>

// Helper function to test for RuntimeError during the loading and interpretation process.
void expect_runtime_error(const std::string& source, const std::string& test_name, const char* expected_message) {
    const std::string filename = "runtime_error_" + test_name + ".yini";
    std::ofstream outfile(filename);
    outfile << source;
    outfile.close();

    YINI::YiniManager manager;
    try {
        manager.load(filename);
        FAIL() << "Expected RuntimeError for source:\n" << source;
    } catch (const YINI::RuntimeError& e) {
        // We can check the error message if we want to be more specific.
        EXPECT_STREQ(e.what(), expected_message) << "Incorrect message for test: " << test_name;
    } catch (const std::exception& e) {
        FAIL() << "Expected RuntimeError but got std::exception: " << e.what();
    } catch (...) {
        FAIL() << "Expected RuntimeError but got an unknown exception.";
    }
}

TEST(ErrorHandlingTest, ThrowsOnCircularSectionInheritance) {
    std::string source = "[A]:B\nkeyA=1\n\n[B]:A\nkeyB=2\n";
    expect_runtime_error(source, "circular_inheritance", "Circular inheritance detected involving section 'A'.");
}

TEST(ErrorHandlingTest, ThrowsOnSectionRedefinition) {
    std::string source = "[A]\nkey=1\n[A]\nkey=2\n";
    expect_runtime_error(source, "section_redefinition", "Section 'A' has already been defined.");
}

TEST(ErrorHandlingTest, ThrowsOnUndefinedMacro) {
    std::string source = "[Test]\nkey = @undefined_macro";
    expect_runtime_error(source, "undefined_macro", "Undefined variable 'undefined_macro'.");
}

TEST(ErrorHandlingTest, ThrowsOnTypeMismatchInExpression) {
    std::string source = "[Test]\nkey = 5 * \"hello\"";
    expect_runtime_error(source, "type_mismatch", "Operands must be numbers.");
}

TEST(ErrorHandlingTest, ThrowsOnTypeMismatchWithMacro) {
    std::string source = "[#define]\nmy_macro = \"not a number\"\n[Test]\nkey = 5 * @my_macro";
    expect_runtime_error(source, "type_mismatch_macro", "Operands must be numbers.");
}