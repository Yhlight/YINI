#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include "gtest/gtest.h"
#include <any>
#include <cstdlib>

TEST(ValidatorTests, ThrowsOnMissingRequiredKey)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, e\n\n[MyConfig]\n";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, AppliesDefaultValueForMissingKey)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, =42\n\n[MyConfig]\n";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
    ASSERT_EQ(config.count("MyConfig.my_key"), 1);
    EXPECT_EQ(std::get<int64_t>(config["MyConfig.my_key"]), 42);
}

TEST(ValidatorTests, AppliesHexDefaultValueForMissingKey)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, =0xFF\n\n[MyConfig]\n";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
    ASSERT_EQ(config.count("MyConfig.my_key"), 1);
    EXPECT_EQ(std::get<int64_t>(config["MyConfig.my_key"]), 255);
}

TEST(ValidatorTests, ThrowsOnTypeMismatch)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, string\n\n[MyConfig]\nmy_key = 123";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, ThrowsOnMinRangeViolation)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, min=10\n\n[MyConfig]\nmy_key = 5";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, ThrowsOnMaxRangeViolation)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, max=20\n\n[MyConfig]\nmy_key = 25";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, PassesWithCorrectValue)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, min=10, max=20\n\n[MyConfig]\nmy_key = 15";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
}

TEST(ValidatorTests, ThrowsOnValidatorArraySubtypeMismatch)
{
    // This test is different from the one above. The resolver will happily resolve
    // `my_array` because it contains all integers. However, the *validator* should
    // fail because the schema expects an array of strings.
    std::string source = R"([#schema]
[MyConfig]
my_array = !, array[string]

[MyConfig]
my_array = [1, 2, 3]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, PassesWithCorrectValidatorArraySubtype)
{
    std::string source = R"([#schema]
[MyConfig]
my_array = !, array[string]

[MyConfig]
my_array = ["one", "two", "three"]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
}

TEST(ValidatorTests, HandlesCombinedRules)
{
    // This test checks an optional key with a default value that is within the specified range.
    std::string source = "[#schema]\n[MyConfig]\nmy_key = ?, int, =15, min=10, max=20\n\n[MyConfig]\n";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
    ASSERT_EQ(config.count("MyConfig.my_key"), 1);
    EXPECT_EQ(std::get<int64_t>(config["MyConfig.my_key"]), 15);
}

TEST(ValidatorTests, ThrowsWhenDefaultValueIsOutOfRange)
{
    // The default value of 5 violates the min=10 rule.
    std::string source = "[#schema]\n[MyConfig]\nmy_key = ?, int, =5, min=10, max=20\n\n[MyConfig]\n";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_THROW(validator.validate(), std::runtime_error);
}

TEST(ValidatorTests, ThrowsOnNestedArraySubtypeMismatch)
{
    std::string source = R"([#schema]
[MyConfig]
my_nested_array = !, array[array[int]]

[MyConfig]
my_nested_array = [[1, 2], [3, "four"]]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);

    EXPECT_THROW(resolver.resolve(), std::runtime_error);
}

TEST(ValidatorTests, PassesWithCorrectNestedArraySubtype)
{
    std::string source = R"([#schema]
[MyConfig]
my_nested_array = !, array[array[int]]

[MyConfig]
my_nested_array = [[1, 2], [3, 4]]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
}

TEST(ValidatorTests, ThrowsOnArraySubtypeMismatch)
{
    std::string source = R"([#schema]
[MyConfig]
my_array = !, array[int]

[MyConfig]
my_array = [1, 2, "three"]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);

    EXPECT_THROW(resolver.resolve(), std::runtime_error);
}

TEST(ValidatorTests, PassesWithCorrectArraySubtype)
{
    std::string source = R"([#schema]
[MyConfig]
my_array = !, array[int]

[MyConfig]
my_array = [1, 2, 3]
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();
    YINI::Validator validator(config, ast);

    EXPECT_NO_THROW(validator.validate());
}
