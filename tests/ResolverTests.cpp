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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyConfig.value"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["MyConfig.value"]), "hello world");
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

    try {
        resolver.resolve();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("Error at line 2"), std::string::npos);
    }
}

TEST(ResolverTests, ResolvesSingleInheritance)
{
    std::string source = "[Parent]\nkey1 = \"value1\"\n\n[Child : Parent]\nkey2 = \"value2\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Child.key1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key1"]), "value1");
    ASSERT_EQ(config.count("Child.key2"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key2"]), "value2");
}

TEST(ResolverTests, ResolvesMultipleInheritance)
{
    std::string source = "[Parent1]\nkey1 = \"p1\"\nkey2 = \"p1\"\n\n[Parent2]\nkey2 = \"p2\"\n\n[Child : Parent1, Parent2]\nkey3 = \"c\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Child.key1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key1"]), "p1");
    ASSERT_EQ(config.count("Child.key2"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key2"]), "p2"); // Parent2 overrides Parent1
    ASSERT_EQ(config.count("Child.key3"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key3"]), "c");
}

TEST(ResolverTests, ResolvesInheritanceWithOverride)
{
    std::string source = "[Parent]\nkey1 = \"parent\"\nkey2 = \"parent\"\n\n[Child : Parent]\nkey2 = \"child\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Child.key1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key1"]), "parent");
    ASSERT_EQ(config.count("Child.key2"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key2"]), "child"); // Child overrides Parent
}

TEST(ResolverTests, ResolvesMultiLevelInheritance)
{
    std::string source = "[Grandparent]\nkey1 = \"gp\"\nkey2 = \"gp\"\n\n[Parent : Grandparent]\nkey2 = \"p\"\n\n[Child : Parent]\nkey3 = \"c\"";
    YINI::Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    YINI::Parser parser(tokens);
    auto ast = parser.parse();
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Child.key1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key1"]), "gp");
    ASSERT_EQ(config.count("Child.key2"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key2"]), "p");
    ASSERT_EQ(config.count("Child.key3"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["Child.key3"]), "c");
}

TEST(ResolverTests, ResolvesSet)
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
    auto set_any = config["MySet.values"];
    ASSERT_EQ(set_any.type(), typeid(std::vector<std::any>));
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("MyMap.data"), 1);
    auto map_any = config["MyMap.data"];
    auto map_val = std::any_cast<std::map<std::string, std::any>>(map_any);
    ASSERT_EQ(map_val.size(), 2);
    ASSERT_EQ(map_val.count("key1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(map_val.at("key1")), "value1");
    ASSERT_EQ(map_val.count("key2"), 1);
    EXPECT_EQ(std::any_cast<double>(map_val.at("key2")), 123.0);
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
    auto color_any = config["Colors.my_color"];
    ASSERT_EQ(color_any.type(), typeid(YINI::ResolvedColor));
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Colors.my_color"), 1);
    auto color_any = config["Colors.my_color"];
    ASSERT_EQ(color_any.type(), typeid(YINI::ResolvedColor));
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto coord_any = config["Coords.pos"];
    ASSERT_EQ(coord_any.type(), typeid(YINI::ResolvedCoord));
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Coords.pos"), 1);
    auto coord_any = config["Coords.pos"];
    ASSERT_EQ(coord_any.type(), typeid(YINI::ResolvedCoord));
    auto coord_val = std::any_cast<YINI::ResolvedCoord>(coord_any);
    EXPECT_EQ(std::any_cast<double>(coord_val.x), 10.0);
    EXPECT_EQ(std::any_cast<double>(coord_val.y), 20.0);
    ASSERT_EQ(coord_val.z.has_value(), true);
    EXPECT_EQ(std::any_cast<double>(coord_val.z), 30.0);
}

TEST(ResolverTests, ResolvesDynaValue)
{
    YINI::YmetaManager ymeta_manager;

    // First run: resolve and cache the initial value
    std::string source1 = "[MyConfig]\nvalue = Dyna(123)";
    YINI::Lexer lexer1(source1);
    auto tokens1 = lexer1.scan_tokens();
    YINI::Parser parser1(tokens1);
    auto ast1 = parser1.parse();
    YINI::Resolver resolver1(ast1, ymeta_manager);
    auto config1 = resolver1.resolve();

    ASSERT_EQ(config1.count("MyConfig.value"), 1);
    EXPECT_EQ(std::any_cast<double>(config1["MyConfig.value"]), 123.0);
    EXPECT_TRUE(ymeta_manager.has_value("MyConfig.value"));

    // Second run: value should be loaded from ymeta, not from the source
    std::string source2 = "[MyConfig]\nvalue = Dyna(456)";
    YINI::Lexer lexer2(source2);
    auto tokens2 = lexer2.scan_tokens();
    YINI::Parser parser2(tokens2);
    auto ast2 = parser2.parse();
    YINI::Resolver resolver2(ast2, ymeta_manager);
    auto config2 = resolver2.resolve();

    ASSERT_EQ(config2.count("MyConfig.value"), 1);
    EXPECT_EQ(std::any_cast<double>(config2["MyConfig.value"]), 123.0); // Should still be the old value

    // Update the value and check backup
    ymeta_manager.set_value("MyConfig.value", 789.0);
    EXPECT_TRUE(ymeta_manager.has_value("MyConfig.value"));
    EXPECT_EQ(std::any_cast<double>(ymeta_manager.get_value("MyConfig.value")), 789.0);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
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
    YINI::YmetaManager ymeta_manager;
    YINI::Resolver resolver(ast, ymeta_manager);
    auto config = resolver.resolve();

    ASSERT_EQ(config.count("Config.value"), 1);
    ASSERT_EQ(std::any_cast<double>(config["Config.value"]), 9.0);
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
    EXPECT_EQ(std::any_cast<std::string>(config["MyConfig.my_path"]), "/usr/local/bin");
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
    auto list_any = config["MyConfig.my_list"];
    auto list_vec = std::any_cast<std::vector<std::any>>(list_any);
    ASSERT_EQ(list_vec.size(), 2);
    EXPECT_EQ(std::any_cast<double>(list_vec[0]), 1.0);
    EXPECT_EQ(std::any_cast<std::string>(list_vec[1]), "two");
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
    EXPECT_EQ(std::any_cast<double>(config["MyReg.0"]), 1.0);
    ASSERT_EQ(config.count("MyReg.1"), 1);
    EXPECT_EQ(std::any_cast<std::string>(config["MyReg.1"]), "two");
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

    try {
        resolver.resolve();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("Error at line 2"), std::string::npos);
    }
}