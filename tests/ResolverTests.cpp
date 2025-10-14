#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include <variant>

TEST(ResolverTests, ResolvesMacro)
{
    std::string source = "[#define]\nmy_macro = \"hello world\"\n\n[MyConfig]\nvalue = @my_macro";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.value"), 1);
    EXPECT_EQ(std::get<std::string>(config["MyConfig.value"]), "hello world");
}

TEST(ResolverTests, ThrowsOnUndefinedMacro)
{
    std::string source = "[MyConfig]\nvalue = @undefined_macro";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);

    ASSERT_THROW(resolver.resolve(), std::runtime_error);
}

TEST(ResolverTests, ResolvesComplexArithmetic)
{
    std::string source = "[Config]\nvalue = (10 - 5) * -2 + 10 / 2.0"; // 5 * -2 + 5 = -10 + 5 = -5
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_TRUE(std::holds_alternative<double>(config["Config.value"]));
    EXPECT_EQ(std::get<double>(config["Config.value"]), -5.0);
}

TEST(ResolverTests, ResolvesSectionInheritance)
{
    std::string source = R"([Parent]
key1 = "value1"
key2 = "original_value"

[Child] : Parent
key2 = "overridden_value"
key3 = "value3")";

    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Child.key1"), 1);
    EXPECT_EQ(std::get<std::string>(config["Child.key1"]), "value1");

    ASSERT_EQ(config.count("Child.key2"), 1);
    EXPECT_EQ(std::get<std::string>(config["Child.key2"]), "overridden_value");

    ASSERT_EQ(config.count("Child.key3"), 1);
    EXPECT_EQ(std::get<std::string>(config["Child.key3"]), "value3");
}

TEST(ResolverTests, ResolvesSetAsArray)
{
    std::string source = "[MySet]\nvalues = (1, \"two\", 3.0)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MySet.values"), 1);
    auto &set_variant = config["MySet.values"];
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(set_variant));
    auto &set_vec = *std::get<std::unique_ptr<YINI::YiniArray>>(set_variant);
    ASSERT_EQ(set_vec.size(), 3);
    EXPECT_EQ(std::get<int64_t>(set_vec[0]), 1);
    EXPECT_EQ(std::get<std::string>(set_vec[1]), "two");
    EXPECT_EQ(std::get<int64_t>(set_vec[2]), 3);
}

TEST(ResolverTests, ResolvesCrossSectionReference)
{
    std::string source = "[Source]\nvalue = \"hello\"\n[Target]\nref = @{Source.value}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Target.ref"), 1);
    EXPECT_EQ(std::get<std::string>(config["Target.ref"]), "hello");
}

TEST(ResolverTests, ResolvesMap)
{
    std::string source = "[MyMap]\ndata = {key1: \"value1\", key2: 123}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyMap.data"), 1);
    auto &map_variant = config["MyMap.data"];
    ASSERT_TRUE(std::holds_alternative<YINI::YiniMap>(map_variant));
    auto &map_val = std::get<YINI::YiniMap>(map_variant);
    ASSERT_EQ(map_val.size(), 2);
    ASSERT_EQ(map_val.count("key1"), 1);
    EXPECT_EQ(std::get<std::string>(map_val.at("key1")), "value1");
    ASSERT_EQ(map_val.count("key2"), 1);
    EXPECT_EQ(std::get<int64_t>(map_val.at("key2")), 123);
}

TEST(ResolverTests, ResolvesHexColor)
{
    std::string source = "[Colors]\nmy_color = #FFC0CB";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Colors.my_color"), 1);
    auto &color_variant = config["Colors.my_color"];
    ASSERT_TRUE(std::holds_alternative<YINI::ResolvedColor>(color_variant));
    auto color_val = std::get<YINI::ResolvedColor>(color_variant);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Colors.my_color"), 1);
    auto &color_variant = config["Colors.my_color"];
    ASSERT_TRUE(std::holds_alternative<YINI::ResolvedColor>(color_variant));
    auto color_val = std::get<YINI::ResolvedColor>(color_variant);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto &coord_variant = config["Coords.pos"];
    ASSERT_TRUE(std::holds_alternative<YINI::ResolvedCoord>(coord_variant));
    auto coord_val = std::get<YINI::ResolvedCoord>(coord_variant);
    EXPECT_EQ(coord_val.x, 10.0);
    EXPECT_EQ(coord_val.y, 20.0);
    EXPECT_EQ(coord_val.has_z, false);
}

TEST(ResolverTests, ResolvesCoord3D)
{
    std::string source = "[Coords]\npos = coord(10, 20, 30)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto &coord_variant = config["Coords.pos"];
    ASSERT_TRUE(std::holds_alternative<YINI::ResolvedCoord>(coord_variant));
    auto coord_val = std::get<YINI::ResolvedCoord>(coord_variant);
    EXPECT_EQ(coord_val.x, 10.0);
    EXPECT_EQ(coord_val.y, 20.0);
    ASSERT_EQ(coord_val.has_z, true);
    EXPECT_EQ(coord_val.z, 30.0);
}

TEST(ResolverTests, ResolvesEnvVar)
{
    // Set an environment variable for the test
    const char *var_name = "YINI_TEST_VAR";
    const char *var_value = "hello from env";
    setenv(var_name, var_value, 1);

    std::string source = "[MyConfig]\nvalue = ${YINI_TEST_VAR}";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.value"), 1);
    EXPECT_EQ(std::get<std::string>(config["MyConfig.value"]), var_value);

    // Clean up the environment variable
    unsetenv(var_name);
}

TEST(ResolverTests, ResolvesInclude)
{
    std::string source =
        "[#include]\n+= \"include_test.yini\"\n[MainSection]\nmain_key = \"this is from the main file\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("IncludedSection.included_key"), 1);
    EXPECT_EQ(std::get<std::string>(config["IncludedSection.included_key"]), "this value is from the included file");

    ASSERT_EQ(config.count("IncludedSection.another_key"), 1);
    EXPECT_EQ(std::get<int64_t>(config["IncludedSection.another_key"]), 123);

    ASSERT_EQ(config.count("MainSection.main_key"), 1);
    EXPECT_EQ(std::get<std::string>(config["MainSection.main_key"]), "this is from the main file");
}

TEST(ResolverTests, ResolvesArithmetic)
{
    std::string source = "[Config]\nvalue = 1 + 2 * 3"; // 1 + 6 = 7
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_TRUE(std::holds_alternative<double>(config["Config.value"]));
    EXPECT_EQ(std::get<double>(config["Config.value"]), 7.0);
}

TEST(ResolverTests, HandlesQuickRegistrationWithInheritance)
{
    std::string source = R"(
[Parent]
0 = "zero"
1 = "one"

[Child] : Parent
+= "two"
)";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_TRUE(config.count("Child.2"));
    EXPECT_EQ(std::get<std::string>(config["Child.2"]), "two");
    ASSERT_TRUE(config.count("Child.0"));
    EXPECT_EQ(std::get<std::string>(config["Child.0"]), "zero");
}

TEST(ResolverTests, ResolvesGroupedArithmetic)
{
    std::string source = "[Config]\nvalue = (1 + 2) * 3"; // 3 * 3 = 9
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_TRUE(std::holds_alternative<double>(config["Config.value"]));
    EXPECT_EQ(std::get<double>(config["Config.value"]), 9.0);
}

TEST(ResolverTests, ResolvesPath)
{
    std::string source = "[MyConfig]\nmy_path = path(\"/usr/local/bin\")";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.my_path"), 1);
    EXPECT_EQ(std::get<std::string>(config["MyConfig.my_path"]), "/usr/local/bin");
}

TEST(ResolverTests, ResolvesList)
{
    std::string source = "[MyConfig]\nmy_list = list(1, \"two\")";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.my_list"), 1);
    auto &list_variant = config["MyConfig.my_list"];
    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<YINI::YiniArray>>(list_variant));
    auto &list_vec = *std::get<std::unique_ptr<YINI::YiniArray>>(list_variant);
    ASSERT_EQ(list_vec.size(), 2);
    EXPECT_EQ(std::get<int64_t>(list_vec[0]), 1);
    EXPECT_EQ(std::get<std::string>(list_vec[1]), "two");
}

TEST(ResolverTests, ResolvesQuickRegistration)
{
    std::string source = "[MyReg]\n+= 1\n+= \"two\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyReg.0"), 1);
    EXPECT_EQ(std::get<int64_t>(config["MyReg.0"]), 1);
    ASSERT_EQ(config.count("MyReg.1"), 1);
    EXPECT_EQ(std::get<std::string>(config["MyReg.1"]), "two");
}

TEST(ResolverTests, ThrowsOnDivisionByZero)
{
    std::string source = "[Config]\nvalue = 1 / 0";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);

    ASSERT_THROW(resolver.resolve(), std::runtime_error);
}
