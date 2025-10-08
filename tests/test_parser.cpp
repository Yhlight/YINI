#include <gtest/gtest.h>
#include "Parser/parser.h"
#include "Lexer/lexer.h"

TEST(ParserTest, ParseSectionAndKeyValuePair) {
    std::string input = "[Config]\nkey = \"value\"";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Config"));
    EXPECT_EQ(std::get<std::string>(config["Config"]["key"]), "value");
}

TEST(ParserTest, ParseInteger) {
    std::string input = "[Numbers]\nint_val = 123";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_EQ(std::get<int>(config["Numbers"]["int_val"]), 123);
}

TEST(ParserTest, ParseFloat) {
    std::string input = "[Numbers]\nfloat_val = 3.14";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Numbers"));
    EXPECT_DOUBLE_EQ(std::get<double>(config["Numbers"]["float_val"]), 3.14);
}

TEST(ParserTest, ParseBoolean) {
    std::string input = "[Booleans]\nbool_true = true\nbool_false = false";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Booleans"));
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_true"]), true);
    EXPECT_EQ(std::get<bool>(config["Booleans"]["bool_false"]), false);
}

TEST(ParserTest, ParseArray) {
    std::string input = "[Data]\narray_val = [1, \"two\", 3.14, true]";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Data"));

    auto& arr_variant = config["Data"]["array_val"];
    auto* arr_ptr = std::get<std::unique_ptr<Array>>(arr_variant).get();

    ASSERT_EQ(arr_ptr->elements.size(), 4);
    EXPECT_EQ(std::get<int>(arr_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<std::string>(arr_ptr->elements[1]), "two");
    EXPECT_DOUBLE_EQ(std::get<double>(arr_ptr->elements[2]), 3.14);
    EXPECT_EQ(std::get<bool>(arr_ptr->elements[3]), true);
}

TEST(ParserTest, ParseNestedArray) {
    std::string input = "[Data]\nnested_array = [[1, 2], [3, 4]]";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Data"));

    auto& nested_arr_variant = config["Data"]["nested_array"];
    auto* nested_arr_ptr = std::get<std::unique_ptr<Array>>(nested_arr_variant).get();

    ASSERT_EQ(nested_arr_ptr->elements.size(), 2);

    // Check first nested array
    auto* inner_arr_1_ptr = std::get<std::unique_ptr<Array>>(nested_arr_ptr->elements[0]).get();
    ASSERT_EQ(inner_arr_1_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<int>(inner_arr_1_ptr->elements[0]), 1);
    EXPECT_EQ(std::get<int>(inner_arr_1_ptr->elements[1]), 2);

    // Check second nested array
    auto* inner_arr_2_ptr = std::get<std::unique_ptr<Array>>(nested_arr_ptr->elements[1]).get();
    ASSERT_EQ(inner_arr_2_ptr->elements.size(), 2);
    EXPECT_EQ(std::get<int>(inner_arr_2_ptr->elements[0]), 3);
    EXPECT_EQ(std::get<int>(inner_arr_2_ptr->elements[1]), 4);
}

TEST(ParserTest, SectionInheritance) {
    std::string input = R"(
[Base1]
key1 = "value1"
key2 = 100

[Base2]
key2 = 200
key3 = true

[Derived] : Base1, Base2
key4 = "derived_value"
)";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Derived"));
    auto& derived_section = config["Derived"];

    // Inherited from Base1
    EXPECT_EQ(std::get<std::string>(derived_section["key1"]), "value1");
    // Overridden by Base2
    EXPECT_EQ(std::get<int>(derived_section["key2"]), 200);
    // Inherited from Base2
    EXPECT_EQ(std::get<bool>(derived_section["key3"]), true);
    // Defined in Derived
    EXPECT_EQ(std::get<std::string>(derived_section["key4"]), "derived_value");
}

TEST(ParserTest, QuickRegistration) {
    std::string input = R"(
[Registry]
+= "item1"
+= 123
+= false
)";
    Lexer lexer(input);
    Parser parser(lexer);

    auto config = parser.parse();

    ASSERT_TRUE(config.count("Registry"));
    auto& registry_section = config["Registry"];

    // The spec is ambiguous on the key for the registration list.
    // We'll assume it's stored under a special, empty key for now.
    ASSERT_TRUE(registry_section.count(""));
    auto& reg_variant = registry_section.at("");
    auto* reg_ptr = std::get<std::unique_ptr<Array>>(reg_variant).get();

    ASSERT_EQ(reg_ptr->elements.size(), 3);
    EXPECT_EQ(std::get<std::string>(reg_ptr->elements[0]), "item1");
    EXPECT_EQ(std::get<int>(reg_ptr->elements[1]), 123);
    EXPECT_EQ(std::get<bool>(reg_ptr->elements[2]), false);
}