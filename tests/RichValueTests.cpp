#include "doctest/doctest.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Parser/Ast.h"
#include "Resolver/Resolver.h"

#include <vector>
#include <string>
#include <memory>

// Helper to parse a string and get the first value from the first section.
std::unique_ptr<Yini::Value> getFirstValue(const std::string& source) {
    Yini::Lexer lexer(source);
    std::vector<Yini::Token> tokens = lexer.scanTokens();
    Yini::Parser parser(tokens);
    auto ast = parser.parse();
    if (ast.empty() || ast[0]->pairs.empty()) {
        return nullptr;
    }
    // We need to clone it because the ast will be destroyed when the vector goes out of scope
    return ast[0]->pairs[0]->value->clone();
}

TEST_CASE("Rich Value Types") {
    SUBCASE("String Value") {
        std::string source = R"([Test] key = "hello world")";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::String);
        auto* stringValue = dynamic_cast<Yini::StringValue*>(value.get());
        REQUIRE(stringValue != nullptr);
        CHECK(stringValue->value == "hello world");
    }

    SUBCASE("Integer Value") {
        std::string source = R"([Test] key = 123)";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Number);
        auto* numValue = dynamic_cast<Yini::NumberValue*>(value.get());
        REQUIRE(numValue != nullptr);
        CHECK(numValue->value == 123.0);
    }

    SUBCASE("Floating-Point Value") {
        std::string source = R"([Test] key = 3.14)";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Number);
        auto* numValue = dynamic_cast<Yini::NumberValue*>(value.get());
        REQUIRE(numValue != nullptr);
        CHECK(numValue->value == 3.14);
    }

    SUBCASE("Boolean True Value") {
        std::string source = R"([Test] key = true)";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Bool);
        auto* boolValue = dynamic_cast<Yini::BoolValue*>(value.get());
        REQUIRE(boolValue != nullptr);
        CHECK(boolValue->value == true);
    }

    SUBCASE("Boolean False Value") {
        std::string source = R"([Test] key = false)";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Bool);
        auto* boolValue = dynamic_cast<Yini::BoolValue*>(value.get());
        REQUIRE(boolValue != nullptr);
        CHECK(boolValue->value == false);
    }

    SUBCASE("Array Value") {
        std::string source = R"([Test] key = [1, "two", true, [3, 4]])";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Array);
        auto* arrayValue = dynamic_cast<Yini::ArrayValue*>(value.get());
        REQUIRE(arrayValue != nullptr);
        REQUIRE(arrayValue->elements.size() == 4);

        // Check element 1: Number
        auto* elem1 = dynamic_cast<Yini::NumberValue*>(arrayValue->elements[0].get());
        REQUIRE(elem1 != nullptr);
        CHECK(elem1->value == 1.0);

        // Check element 2: String
        auto* elem2 = dynamic_cast<Yini::StringValue*>(arrayValue->elements[1].get());
        REQUIRE(elem2 != nullptr);
        CHECK(elem2->value == "two");

        // Check element 3: Bool
        auto* elem3 = dynamic_cast<Yini::BoolValue*>(arrayValue->elements[2].get());
        REQUIRE(elem3 != nullptr);
        CHECK(elem3->value == true);

        // Check element 4: Nested Array
        auto* elem4 = dynamic_cast<Yini::ArrayValue*>(arrayValue->elements[3].get());
        REQUIRE(elem4 != nullptr);
        REQUIRE(elem4->elements.size() == 2);
        auto* nestedElem1 = dynamic_cast<Yini::NumberValue*>(elem4->elements[0].get());
        REQUIRE(nestedElem1 != nullptr);
        CHECK(nestedElem1->value == 3.0);
    }

    SUBCASE("List(...) Value") {
        std::string source = R"([Test] key = List(1, "two", true))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Array);
        auto* arrayValue = dynamic_cast<Yini::ArrayValue*>(value.get());
        REQUIRE(arrayValue != nullptr);
        REQUIRE(arrayValue->elements.size() == 3);

        auto* elem1 = dynamic_cast<Yini::NumberValue*>(arrayValue->elements[0].get());
        CHECK(elem1->value == 1.0);
        auto* elem2 = dynamic_cast<Yini::StringValue*>(arrayValue->elements[1].get());
        CHECK(elem2->value == "two");
        auto* elem3 = dynamic_cast<Yini::BoolValue*>(arrayValue->elements[2].get());
        CHECK(elem3->value == true);
    }

    SUBCASE("Array(...) Value") {
        std::string source = R"([Test] key = Array(1, "two", true))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Array);
        auto* arrayValue = dynamic_cast<Yini::ArrayValue*>(value.get());
        REQUIRE(arrayValue != nullptr);
        REQUIRE(arrayValue->elements.size() == 3);
    }

    SUBCASE("Set Value") {
        std::string source = R"([Test] key = (1, "two", true))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Set);
        auto* setValue = dynamic_cast<Yini::SetValue*>(value.get());
        REQUIRE(setValue != nullptr);
        REQUIRE(setValue->elements.size() == 3);

        auto* elem1 = dynamic_cast<Yini::NumberValue*>(setValue->elements[0].get());
        CHECK(elem1->value == 1.0);
        auto* elem2 = dynamic_cast<Yini::StringValue*>(setValue->elements[1].get());
        CHECK(elem2->value == "two");
        auto* elem3 = dynamic_cast<Yini::BoolValue*>(setValue->elements[2].get());
        CHECK(elem3->value == true);
    }

    SUBCASE("Map Value") {
        std::string source = R"([Test] key = { a: 1, b: "two", c: true })";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Map);
        auto* mapValue = dynamic_cast<Yini::MapValue*>(value.get());
        REQUIRE(mapValue != nullptr);
        REQUIRE(mapValue->elements.size() == 3);

        auto* elem1 = dynamic_cast<Yini::NumberValue*>(mapValue->elements["a"].get());
        CHECK(elem1->value == 1.0);
        auto* elem2 = dynamic_cast<Yini::StringValue*>(mapValue->elements["b"].get());
        CHECK(elem2->value == "two");
        auto* elem3 = dynamic_cast<Yini::BoolValue*>(mapValue->elements["c"].get());
        CHECK(elem3->value == true);
    }

    SUBCASE("Hex Color Value") {
        std::string source = R"([Test] key = #FF00FF)";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Color);
        auto* colorValue = dynamic_cast<Yini::ColorValue*>(value.get());
        REQUIRE(colorValue != nullptr);
        CHECK(colorValue->r == 255);
        CHECK(colorValue->g == 0);
        CHECK(colorValue->b == 255);
        CHECK(colorValue->a == 255);
    }

    SUBCASE("Function Color Value") {
        std::string source = R"([Test] key = color(10, 20, 30, 40))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Color);
        auto* colorValue = dynamic_cast<Yini::ColorValue*>(value.get());
        REQUIRE(colorValue != nullptr);
        CHECK(colorValue->r == 10);
        CHECK(colorValue->g == 20);
        CHECK(colorValue->b == 30);
        CHECK(colorValue->a == 40);
    }

    SUBCASE("Coord Value") {
        std::string source = R"([Test] key = Coord(1.5, 2.5, 3.5))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Coord);
        auto* coordValue = dynamic_cast<Yini::CoordValue*>(value.get());
        REQUIRE(coordValue != nullptr);
        CHECK(coordValue->x == 1.5);
        CHECK(coordValue->y == 2.5);
        CHECK(coordValue->z == 3.5);
        CHECK(coordValue->has_z == true);
    }

    SUBCASE("Path Value") {
        std::string source = R"([Test] key = Path("my/path/to/file.txt"))";
        auto value = getFirstValue(source);
        REQUIRE(value != nullptr);
        REQUIRE(value->getType() == Yini::ValueType::Path);
        auto* pathValue = dynamic_cast<Yini::PathValue*>(value.get());
        REQUIRE(pathValue != nullptr);
        CHECK(pathValue->path == "my/path/to/file.txt");
    }
}