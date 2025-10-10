#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "Validator/Validator.h"
#include <any>
#include <cstdlib>

TEST(ValidatorTests, ThrowsOnMissingRequiredKey)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !\n\n[MyConfig]\n";
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

TEST(ValidatorTests, PassesWithRequiredKeyPresent)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !\n\n[MyConfig]\nmy_key = 123";
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

TEST(ValidatorTests, AssignsDefaultValue)
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
    EXPECT_EQ(std::any_cast<double>(config["MyConfig.my_key"]), 42.0);
}

TEST(ValidatorTests, ThrowsOnRangeViolation)
{
    std::string source = "[#schema]\n[MyConfig]\nmy_key = !, int, min=10, max=20\n\n[MyConfig]\nmy_key = 5";
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