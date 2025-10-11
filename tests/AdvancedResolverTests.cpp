#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "Ymeta/YmetaManager.h"

// Helper function to run the full pipeline and catch expected exceptions
void ExpectPipelineError(const std::string& source, const std::string& expected_error_msg)
{
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);

    try
    {
        auto resolved_config = resolver.resolve();
        YINI::Validator validator(resolved_config, ast);
        validator.validate();
        FAIL() << "Expected a runtime_error to be thrown.";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_NE(std::string(e.what()).find(expected_error_msg), std::string::npos);
    }
}

TEST(AdvancedResolverTests, SchemaValidationFailsOnWrongType)
{
    std::string source =
        "[#schema]\n"
        "[Config]\n"
        "value = !, int\n"
        "[Config]\n"
        "value = \"not an int\"";
    ExpectPipelineError(source, "Type mismatch for key 'value'. Expected number.");
}

TEST(AdvancedResolverTests, SchemaValidationFailsOnOutOfRangeMin)
{
    std::string source =
        "[#schema]\n"
        "[Config]\n"
        "value = !, int, min=10\n"
        "[Config]\n"
        "value = 5";
    ExpectPipelineError(source, "Value for key 'value' is below the minimum of 10");
}

TEST(AdvancedResolverTests, SchemaValidationFailsOnOutOfRangeMax)
{
    std::string source =
        "[#schema]\n"
        "[Config]\n"
        "value = !, int, max=10\n"
        "[Config]\n"
        "value = 15";
    ExpectPipelineError(source, "Value for key 'value' is above the maximum of 10");
}

TEST(AdvancedResolverTests, SchemaValidationFailsOnRequiredKeyMissing)
{
    std::string source =
        "[#schema]\n"
        "[Config]\n"
        "value = !, int, e\n"
        "[Config]\n"
        "another_value = 123";
    ExpectPipelineError(source, "Missing required key 'value' in section 'Config'.");
}

TEST(AdvancedResolverTests, ArithmeticWithReferences)
{
    std::string source =
        "[#define]\n"
        "base_value = 10\n"
        "[Config]\n"
        "value = @base_value * (2 + 3)"; // Expected: 10 * 5 = 50

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto resolved_config = resolver.resolve();

    ASSERT_TRUE(resolved_config.count("Config.value"));
    EXPECT_EQ(std::any_cast<double>(resolved_config["Config.value"]), 50.0);
}

TEST(AdvancedResolverTests, CircularReferenceDetection)
{
    std::string source =
        "[A]\n"
        "value = @{B.value}\n"
        "[B]\n"
        "value = @{A.value}\n";
    ExpectPipelineError(source, "Circular inheritance detected involving section: A");
}
