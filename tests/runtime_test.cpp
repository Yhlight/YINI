#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"
#include "yini.h"
#include <vector>

using namespace Yini;

TEST(RuntimeTest, TestLazyEvaluation) {
    std::string input = R"yini(
[Settings]
c = a + b
a = 10

[Values]
b = 20
)yini";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();
    ASSERT_TRUE(parser.getErrors().empty());

    YiniRuntime runtime;
    runtime.load(doc.get());

    auto val = runtime.getValue("Settings", "c");
    ASSERT_NE(val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(val->data));
    ASSERT_EQ(std::get<Integer>(val->data), 30);
}

TEST(RuntimeTest, TestInheritance) {
    std::string input = R"yini(
[Base]
name = "Base"
value = 100

[Child] : Base
value = 200
extra = true
)yini";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();
    ASSERT_TRUE(parser.getErrors().empty());

    YiniRuntime runtime;
    runtime.load(doc.get());

    // Check inherited value
    auto name_val = runtime.getValue("Child", "name");
    ASSERT_NE(name_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<String>(name_val->data));
    ASSERT_EQ(std::get<String>(name_val->data), "Base");

    // Check overridden value
    auto value_val = runtime.getValue("Child", "value");
    ASSERT_NE(value_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(value_val->data));
    ASSERT_EQ(std::get<Integer>(value_val->data), 200);

    // Check child-specific value
    auto extra_val = runtime.getValue("Child", "extra");
    ASSERT_NE(extra_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Boolean>(extra_val->data));
    ASSERT_EQ(std::get<Boolean>(extra_val->data), true);
}

TEST(RuntimeTest, TestMacroInArithmetic) {
    std::string input = R"yini(
[#define]
multiplier = 10

[Settings]
result = 5 * @multiplier
)yini";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();
    ASSERT_TRUE(parser.getErrors().empty());

    YiniRuntime runtime;
    runtime.load(doc.get());

    auto val = runtime.getValue("Settings", "result");
    ASSERT_NE(val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(val->data));
    ASSERT_EQ(std::get<Integer>(val->data), 50);
}

TEST(RuntimeTest, TestMapEvaluation) {
    std::string input = "data = {{key1: 10 * 2, key2: \"hello\"}}";
    Lexer lexer(input);
    Parser parser(lexer);
    auto doc = parser.parseDocument();
    ASSERT_TRUE(parser.getErrors().empty());

    YiniRuntime runtime;
    runtime.load(doc.get());

    auto val = runtime.getValue("Default", "data");
    ASSERT_NE(val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Map>(val->data));

    auto map = std::get<Map>(val->data);
    ASSERT_EQ(map.size(), 2);

    auto val1 = map["key1"];
    ASSERT_TRUE(std::holds_alternative<Integer>(val1->data));
    ASSERT_EQ(std::get<Integer>(val1->data), 20);

    auto val2 = map["key2"];
    ASSERT_TRUE(std::holds_alternative<String>(val2->data));
    ASSERT_EQ(std::get<String>(val2->data), "hello");
}

TEST(ErrorHandlingTest, TestParsingErrorRetrieval) {
    std::string input = "[MissingBracket";
    YINI_HANDLE handle = yini_load_from_string(input.c_str());
    ASSERT_NE(handle, nullptr);

    int error_count = yini_get_error_count(handle);
    ASSERT_EQ(error_count, 1);

    char buffer[256];
    int line, col;
    yini_get_error_details(handle, 0, buffer, 256, &line, &col);
    // Add assertions for line, col, and message content if desired

    yini_free(handle);
}
