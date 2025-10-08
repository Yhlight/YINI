#include "doctest/doctest.h"
#include "Loader/Loader.h"
#include "Parser/Ast.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

// Helper to get a map of key-value pairs from a section, simplified for testing
std::map<std::string, Yini::Value*> getSectionValueMap(const Yini::SectionNode* section) {
    std::map<std::string, Yini::Value*> pairs;
    if (section) {
        for (const auto& pair : section->pairs) {
            pairs[pair->key.lexeme] = pair->value.get();
        }
    }
    return pairs;
}


TEST_CASE("File Inclusion")
{
    Yini::Loader loader;
    auto ast = loader.load("tests/include_main.yini");

    // After loading, the AST should be a merged representation.
    // Let's find the 'Settings' and 'Graphics' sections.
    Yini::SectionNode* settingsSection = nullptr;
    Yini::SectionNode* graphicsSection = nullptr;

    for(const auto& section : ast) {
        if(section->name.lexeme == "Settings") {
            settingsSection = section.get();
        }
        if(section->name.lexeme == "Graphics") {
            graphicsSection = section.get();
        }
    }

    // Check that both sections exist
    REQUIRE(settingsSection != nullptr);
    REQUIRE(graphicsSection != nullptr);

    // Check the merged 'Settings' section
    auto settingsPairs = getSectionValueMap(settingsSection);
    REQUIRE(settingsPairs.size() == 3);

    // 'volume' should be inherited from the dependency
    auto* volValue = dynamic_cast<Yini::NumberValue*>(settingsPairs["volume"]);
    REQUIRE(volValue != nullptr);
    CHECK(volValue->value == 0.5);

    // 'fullscreen' should be overridden by the main file
    auto* fsValue = dynamic_cast<Yini::BoolValue*>(settingsPairs["fullscreen"]);
    REQUIRE(fsValue != nullptr);
    CHECK(fsValue->value == true);

    // 'user' is unique to the main file, and it's a string
    auto* userValue = dynamic_cast<Yini::StringValue*>(settingsPairs["user"]);
    REQUIRE(userValue != nullptr);
    CHECK(userValue->value == "Jules");

    // Check the merged 'Graphics' section
    auto graphicsPairs = getSectionValueMap(graphicsSection);
    REQUIRE(graphicsPairs.size() == 2);

    // 'quality' should be overridden
    auto* qualityValue = dynamic_cast<Yini::IdentifierValue*>(graphicsPairs["quality"]);
    REQUIRE(qualityValue != nullptr);
    CHECK(qualityValue->token.lexeme == "high");

    // 'vsync' is unique to the main file
    auto* vsyncValue = dynamic_cast<Yini::BoolValue*>(graphicsPairs["vsync"]);
    REQUIRE(vsyncValue != nullptr);
    CHECK(vsyncValue->value == true);
}