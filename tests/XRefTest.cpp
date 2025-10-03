#include <gtest/gtest.h>
#include "Core/YiniManager.h"
#include "Core/YiniException.h"
#include <fstream>

// Helper to create a file and load it with a YiniManager
static void load_from_source(YINI::YiniManager& manager, const std::string& source) {
    std::ofstream outfile("xref_test.yini");
    outfile << source;
    outfile.close();
    manager.load("xref_test.yini");
}

TEST(XRefTest, ResolvesSimpleReferenceInSameSection) {
    YINI::YiniManager manager;
    std::string source = R"(
        [MySection]
        keyA = "hello"
        keyB = @{MySection.keyA}
    )";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("MySection", "keyB");
    ASSERT_TRUE(std::holds_alternative<std::string>(value.m_value));
    EXPECT_EQ(std::get<std::string>(value.m_value), "hello");
}

TEST(XRefTest, ResolvesReferenceToDifferentSection) {
    YINI::YiniManager manager;
    std::string source = R"(
        [Source]
        value = 123
        [Target]
        ref = @{Source.value} + 7
    )";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("Target", "ref");
    ASSERT_TRUE(std::holds_alternative<double>(value.m_value));
    EXPECT_EQ(std::get<double>(value.m_value), 130);
}

TEST(XRefTest, ResolvesMultiLevelReferences) {
    YINI::YiniManager manager;
    std::string source = R"(
        [A]
        val = 10
        [B]
        val = @{A.val} * 2
        [C]
        val = @{B.val} + 5
    )";
    load_from_source(manager, source);

    YINI::YiniValue value = manager.get_value("C", "val");
    ASSERT_TRUE(std::holds_alternative<double>(value.m_value));
    EXPECT_EQ(std::get<double>(value.m_value), 25);
}

TEST(XRefTest, ThrowsOnCircularReference) {
    YINI::YiniManager manager;
    std::string source = R"(
        [Cycle]
        a = @{Cycle.b}
        b = @{Cycle.a}
    )";

    try {
        load_from_source(manager, source);
        FAIL() << "Expected a RuntimeError for circular reference.";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_PRED_FORMAT2(::testing::IsSubstring, "Circular reference detected", e.what());
    } catch (...) {
        FAIL() << "Expected a RuntimeError, but got a different exception.";
    }
}

TEST(XRefTest, ThrowsOnNonExistentReference) {
    YINI::YiniManager manager;
    std::string source = "[Test]\nkey = @{Bad.ref}";

    try {
        load_from_source(manager, source);
        FAIL() << "Expected a RuntimeError for non-existent reference.";
    } catch (const YINI::RuntimeError& e) {
        EXPECT_STREQ(e.what(), "Referenced key 'ref' not found in section 'Bad'.");
    } catch (...) {
        FAIL() << "Expected a RuntimeError, but got a different exception.";
    }
}