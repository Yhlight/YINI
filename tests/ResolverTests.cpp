#include "gtest/gtest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include <any>
#include <cstdlib>

TEST(ResolverTests, ResolvesMacro)
{
    std::string source = "[#define]\nmy_macro = \"hello world\"\n\n[MyConfig]\nvalue = @my_macro";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.value"), 1);
    ASSERT_EQ(std::any_cast<std::string>(config["MyConfig.value"]), "hello world");
}

TEST(ResolverTests, ThrowsOnUndefinedMacro)
{
    std::string source = "[MyConfig]\nvalue = @undefined_macro";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);

    EXPECT_THROW(resolver.resolve(), std::runtime_error);
}

TEST(ResolverTests, ResolvesSet)
{
    std::string source = "[MySet]\nvalues = (1, \"two\", 3.0)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MySet.values"), 1);
    auto set_any = config["MySet.values"];
    auto set_vec = std::any_cast<std::vector<std::any>>(set_any);
    ASSERT_EQ(set_vec.size(), 3);
    EXPECT_EQ(std::any_cast<double>(set_vec[0]), 1.0);
    EXPECT_EQ(std::any_cast<std::string>(set_vec[1]), "two");
    EXPECT_EQ(std::any_cast<double>(set_vec[2]), 3.0);
}

TEST(ResolverTests, ResolvesCrossSectionReference)
{
    std::string source = "[Source]\nvalue = \"hello\"\n[Target]\nref = @{Source.value}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Target.ref"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Target.ref"]), "hello");
}

TEST(ResolverTests, ResolvesMap)
{
    std::string source = "[MyMap]\ndata = {key1: \"value1\", key2: 123}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyMap.data"), 1);
    auto map_any = config["MyMap.data"];
    auto map_val = std::any_cast<std::map<std::string, std::any>>(map_any);
    ASSERT_EQ(map_val.size(), 2);
    EXPECT_EQ(std::any_cast<std::string>(map_val["key1"]), "value1");
    EXPECT_EQ(std::any_cast<double>(map_val["key2"]), 123.0);
}

TEST(ResolverTests, ResolvesHexColor)
{
    std::string source = "[Colors]\nmy_color = #FFC0CB";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Colors.my_color"), 1);
    auto color_any = config["Colors.my_color"];
    auto color_val = std::any_cast<YINI::ResolvedColor>(color_any);
    EXPECT_EQ(color_val.r, 255);
    EXPECT_EQ(color_val.g, 192);
    EXPECT_EQ(color_val.b, 203);
}

TEST(ResolverTests, ResolvesRgbColor)
{
    std::string source = "[Colors]\nmy_color = color(255, 192, 203)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Colors.my_color"), 1);
    auto color_any = config["Colors.my_color"];
    auto color_val = std::any_cast<YINI::ResolvedColor>(color_any);
    EXPECT_EQ(color_val.r, 255);
    EXPECT_EQ(color_val.g, 192);
    EXPECT_EQ(color_val.b, 203);
}

TEST(ResolverTests, ResolvesCoord2D)
{
    std::string source = "[Coords]\npos = coord(10, 20)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto coord_any = config["Coords.pos"];
    auto coord_val = std::any_cast<YINI::ResolvedCoord>(coord_any);
    EXPECT_EQ(std::any_cast<double>(coord_val.x), 10.0);
    EXPECT_EQ(std::any_cast<double>(coord_val.y), 20.0);
    EXPECT_EQ(coord_val.z.has_value(), false);
}

TEST(ResolverTests, ResolvesCoord3D)
{
    std::string source = "[Coords]\npos = coord(10, 20, 30)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto coord_any = config["Coords.pos"];
    auto coord_val = std::any_cast<YINI::ResolvedCoord>(coord_any);
    EXPECT_EQ(std::any_cast<double>(coord_val.x), 10.0);
    EXPECT_EQ(std::any_cast<double>(coord_val.y), 20.0);
    ASSERT_EQ(coord_val.z.has_value(), true);
    EXPECT_EQ(std::any_cast<double>(coord_val.z), 30.0);
}

TEST(ResolverTests, ResolvesEnvVarReference)
{
    // Set an environment variable for the test
#ifdef _WIN32
    _putenv_s("YINI_TEST_VAR", "hello from env");
#else
    setenv("YINI_TEST_VAR", "hello from env", 1);
#endif

    std::string source = "[Config]\nvalue = ${YINI_TEST_VAR}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Config.value"]), "hello from env");

    // Clean up the environment variable
#ifdef _WIN32
    _putenv_s("YINI_TEST_VAR", "");
#else
    unsetenv("YINI_TEST_VAR");
#endif
}

TEST(ResolverTests, ResolvesInclude)
{
    std::string source = "[#include]\n+= \"include_test.yini\"\n[MainSection]\nmain_key = \"this is from the main file\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("IncludedSection.included_key"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["IncludedSection.included_key"]), "this value is from the included file");

    ASSERT_EQ(config.count("IncludedSection.another_key"), 1);
    EXPECT_EQ(std::any_cast<double>(config["IncludedSection.another_key"]), 123.0);

    ASSERT_EQ(config.count("MainSection.main_key"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["MainSection.main_key"]), "this is from the main file");
}

TEST(ResolverTests, ResolvesArithmetic)
{
    std::string source = "[Config]\nvalue = 1 + 2 * 3"; // 1 + 6 = 7
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_EQ(std::any_cast<double>(config["Config.value"]), 7.0);
}

TEST(ResolverTests, ResolvesGroupedArithmetic)
{
    std::string source = "[Config]\nvalue = (1 + 2) * 3"; // 3 * 3 = 9
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_EQ(std::any_cast<double>(config["Config.value"]), 9.0);
}

TEST(ResolverTests, ThrowsOnDivisionByZero)
{
    std::string source = "[Config]\nvalue = 1 / 0";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::Resolver resolver(ast);

    EXPECT_THROW(resolver.resolve(), std::runtime_error);
}