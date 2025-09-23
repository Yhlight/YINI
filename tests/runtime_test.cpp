#include <gtest/gtest.h>
#include "Parser/Parser.h"
#include "Runtime/Runtime.h"
#include "yini.h"
#include "../src/c_api_internal.h" // For white-box testing
#include <vector>

using namespace Yini;

TEST(RuntimeCorrectionTest, TestNoKeyToKeyReference) {
    std::string input = "a = 10\nb = a"; // This is invalid in the corrected model

    YINI_HANDLE handle = yini_load_from_string(input.c_str());
    ASSERT_NE(handle, nullptr);

    auto* h = static_cast<YiniHandleInternal*>(handle);
    h->runtime->getValue("Default", "b");

    // Expect a runtime error because 'a' is not a macro
    ASSERT_FALSE(h->runtime->getErrors().empty());
    ASSERT_EQ(h->runtime->getErrors()[0].type, ErrorType::Runtime);

    yini_free(handle);
}

TEST(HardFailTest, TestMissingMacro) {
    std::string input = "value = @undefined_macro";
    YINI_HANDLE handle = yini_load_from_string(input.c_str());
    ASSERT_EQ(handle, nullptr);
}

TEST(RuntimeCorrectionTest, TestMacroInArithmetic) {
    std::string input = "[#define]\nmultiplier=10\n[Settings]\nresult=5*@multiplier";

    YINI_HANDLE handle = yini_load_from_string(input.c_str());
    ASSERT_NE(handle, nullptr);

    auto* h = static_cast<YiniHandleInternal*>(handle);
    auto val = h->runtime->getValue("Settings", "result");

    ASSERT_TRUE(h->runtime->getErrors().empty());
    ASSERT_NE(val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(val->data));
    ASSERT_EQ(std::get<Integer>(val->data), 50);

    yini_free(handle);
}

TEST(IncludeTest, TestIncludeAndOverride) {
    // This test relies on the test files created in the tests/ directory
    YINI_HANDLE handle = yini_load_from_file("tests/main_test.yini");
    ASSERT_NE(handle, nullptr);

    auto* h = static_cast<YiniHandleInternal*>(handle);

    // Test that the final macro value is from the last included file
    auto final_val = h->runtime->getValue("Final", "final_val");
    ASSERT_NE(final_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<String>(final_val->data));
    ASSERT_EQ(std::get<String>(final_val->data), "Hello from include2");

    // Test that inheritance and overrides work across included files
    auto override_val = h->runtime->getValue("Final", "final_override");
    ASSERT_NE(override_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(override_val->data));
    ASSERT_EQ(std::get<Integer>(override_val->data), 201); // 200 from include2 + 1

    // Test that a base value is inherited
    auto base_val = h->runtime->getValue("Final", "val1");
    ASSERT_NE(base_val, nullptr);
    ASSERT_TRUE(std::holds_alternative<Integer>(base_val->data));
    ASSERT_EQ(std::get<Integer>(base_val->data), 1);

    yini_free(handle);
}
