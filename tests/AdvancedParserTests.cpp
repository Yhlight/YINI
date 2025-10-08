#include "doctest/doctest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/Ast.h"
#include "Resolver/Resolver.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

// A helper function to create a YiniDocument from source and return the resolved AST
std::vector<std::unique_ptr<Yini::SectionNode>> parseAndResolve(const std::string& source) {
    Yini::Lexer lexer(source);
    std::vector<Yini::Token> tokens = lexer.scanTokens();
    Yini::Parser parser(tokens);
    auto ast = parser.parse();
    Yini::Resolver resolver(ast);
    resolver.resolve();
    return ast;
}

// Helper to get a map of key-value pairs from a section
std::map<std::string, std::string> getSectionPairs(const Yini::SectionNode* section) {
    std::map<std::string, std::string> pairs;
    if (section) {
        for (const auto& pair : section->pairs) {
            pairs[pair->key.lexeme] = pair->value->token.lexeme;
        }
    }
    return pairs;
}


TEST_CASE("Section Inheritance Logic")
{
    std::string source = R"(
[Parent1]
key1 = value1
key2 = value2

[Parent2]
key2 = overridden
key3 = value3

[Child] : Parent1, Parent2
key4 = value4
key1 = child_override
)";

    auto ast = parseAndResolve(source);

    // Find the Child section
    Yini::SectionNode* childSection = nullptr;
    for(const auto& section : ast) {
        if(section->name.lexeme == "Child") {
            childSection = section.get();
            break;
        }
    }

    REQUIRE(childSection != nullptr);

    auto pairs = getSectionPairs(childSection);

    // Total keys should be 4 (key1, key2, key3, key4)
    CHECK(pairs.size() == 4);

    // Check inherited and overridden values
    // Child's own value for key1 should take precedence
    CHECK(pairs["key1"] == "child_override");
    // Parent2's value for key2 should override Parent1's
    CHECK(pairs["key2"] == "overridden");
    // key3 should be inherited from Parent2
    CHECK(pairs["key3"] == "value3");
    // key4 is the child's own key
    CHECK(pairs["key4"] == "value4");
}

TEST_CASE("Quick Registration")
{
    std::string source = R"(
[Registry]
+= item1
+= item2
key = value
+= item3
)";

    auto ast = parseAndResolve(source);

    Yini::SectionNode* registrySection = nullptr;
    for(const auto& section : ast) {
        if(section->name.lexeme == "Registry") {
            registrySection = section.get();
            break;
        }
    }

    REQUIRE(registrySection != nullptr);

    auto pairs = getSectionPairs(registrySection);

    CHECK(pairs.size() == 4);
    CHECK(pairs["0"] == "item1");
    CHECK(pairs["1"] == "item2");
    CHECK(pairs["2"] == "item3");
    CHECK(pairs["key"] == "value");
}