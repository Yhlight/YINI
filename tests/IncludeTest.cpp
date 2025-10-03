#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include <fstream>
#include <vector>
#include <variant>

TEST(IncludeTest, HandlesFileInclusionAndMerging)
{
    // Create the test files.
    std::ofstream("include_one.yini") << R"(
        [#define]
        var1 = "from one"

        [Shared]
        key1 = "one"
        key2 = 1
    )";

    std::ofstream("include_two.yini") << R"(
        [#define]
        var2 = "from two"

        [Shared]
        key2 = 2
        key3 = "two"
    )";

    std::ofstream("include_root.yini") << R"(
        [#include]
        += "include_one.yini"
        += "include_two.yini"

        [#define]
        var1 = "from root" // Override var1 from include_one

        [Shared]
        key3 = "root" // Override key3 from include_two
        key4 = "root"

        [Result]
        resolved_var1 = @var1
        resolved_var2 = @var2
    )";

    YINI::YiniManager manager;
    manager.load("include_root.yini");

    const auto& resolved = manager.get_interpreter().resolved_sections;

    // Test merged [Shared] section
    ASSERT_EQ(resolved.count("Shared"), 1);
    const auto& shared_section = resolved.at("Shared");
    EXPECT_EQ(std::get<std::string>(shared_section.at("key1").m_value), "one");
    EXPECT_EQ(std::get<double>(shared_section.at("key2").m_value), 2);
    EXPECT_EQ(std::get<std::string>(shared_section.at("key3").m_value), "root");
    EXPECT_EQ(std::get<std::string>(shared_section.at("key4").m_value), "root");

    // Test merged [#define] macros via the [Result] section
    ASSERT_EQ(resolved.count("Result"), 1);
    const auto& result_section = resolved.at("Result");
    EXPECT_EQ(std::get<std::string>(result_section.at("resolved_var1").m_value), "from root");
    EXPECT_EQ(std::get<std::string>(result_section.at("resolved_var2").m_value), "from two");
}