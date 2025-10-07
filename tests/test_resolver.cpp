#include <memory>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "Lexer/Lexer.h"
#include "Parser/Ast.h"
#include "Parser/Parser.h"
#include "Resolver/Resolver.h"

using namespace YINI;

TEST(ResolverTest, ResolveCrossSectionReferences) {
    std::string source = R"(
        [Config]
        width = 1920
        height = 1080

        [Window]
        w = @{Config.width}
        h = @{Config.height}
    )";

    Lexer lexer(source);
    Parser parser(lexer, "dummy_resolver_test.yini");
    std::unique_ptr<AstNode> ast = parser.parse();

    Resolver resolver;
    resolver.resolve(*ast);

    ASSERT_NE(ast, nullptr);
    const auto& section = ast->sections[1]; // Window section
    ASSERT_EQ(section.key_values.size(), 2);

    // Check resolved width
    const auto& width_kv = section.key_values[0];
    EXPECT_EQ(width_kv.key, "w");
    auto* width_ptr = std::get_if<int64_t>(&width_kv.value->value);
    ASSERT_TRUE(width_ptr);
    EXPECT_EQ(*width_ptr, 1920);

    // Check resolved height
    const auto& height_kv = section.key_values[1];
    EXPECT_EQ(height_kv.key, "h");
    auto* height_ptr = std::get_if<int64_t>(&height_kv.value->value);
    ASSERT_TRUE(height_ptr);
    EXPECT_EQ(*height_ptr, 1080);
}