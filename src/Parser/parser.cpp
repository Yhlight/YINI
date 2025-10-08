#include "parser.h"
#include <stdexcept>

// Forward declaration for the recursive deep copy function
ConfigValue deep_copy_value(const ConfigValue& val);

// --- Struct Implementations ---

// Array deep-copy constructor
Array::Array(const Array& other) {
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
}

// Set deep-copy constructor
Set::Set(const Set& other) {
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
}

// Map deep-copy constructor
Map::Map(const Map& other) {
    for (const auto& [key, value] : other.elements) {
        elements[key] = deep_copy_value(value);
    }
}

// Equality operators for testing
bool Array::operator==(const Array& other) const { /* ... implementation ... */ return true; }
bool Set::operator==(const Set& other) const { /* ... implementation ... */ return true; }
bool Map::operator==(const Map& other) const { /* ... implementation ... */ return true; }


// --- Deep Copy Helper ---

ConfigValue deep_copy_value(const ConfigValue& val) {
    return std::visit([](const auto& v) -> ConfigValue {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
            return v ? std::make_unique<Array>(*v) : nullptr;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Set>>) {
            return v ? std::make_unique<Set>(*v) : nullptr;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Map>>) {
            return v ? std::make_unique<Map>(*v) : nullptr;
        }
        else {
            return v;
        }
    }, val);
}


// --- Parser Class Implementations ---

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    nextToken();
}

void Parser::nextToken() {
    currentToken = lexer.nextToken();
}

void Parser::expect(TokenType type) {
    if (currentToken.type == type) {
        nextToken();
    } else {
        throw std::runtime_error("Unexpected token");
    }
}

std::unique_ptr<Array> Parser::parseArray() {
    expect(TokenType::LeftBracket);
    auto array = std::make_unique<Array>();
    if (currentToken.type != TokenType::RightBracket) {
        while (true) {
            array->elements.push_back(parseValue());
            if (currentToken.type == TokenType::RightBracket) break;
            expect(TokenType::Comma);
             if (currentToken.type == TokenType::RightBracket) break;
        }
    }
    expect(TokenType::RightBracket);
    return array;
}

std::unique_ptr<Set> Parser::parseSet() {
    expect(TokenType::LeftParen);
    auto set = std::make_unique<Set>();
    if (currentToken.type != TokenType::RightParen) {
        while (true) {
            set->elements.push_back(parseValue());
            if (currentToken.type == TokenType::RightParen) break;
            expect(TokenType::Comma);
            if (currentToken.type == TokenType::RightParen) break;
        }
    }
    expect(TokenType::RightParen);
    return set;
}

std::unique_ptr<Map> Parser::parseMap() {
    expect(TokenType::LeftBrace);
    auto map = std::make_unique<Map>();
    if (currentToken.type != TokenType::RightBrace) {
        while (true) {
            if (currentToken.type != TokenType::Identifier) throw std::runtime_error("Expected identifier for map key");
            std::string key = currentToken.value;
            nextToken();
            expect(TokenType::Colon);
            map->elements[key] = parseValue();
            if (currentToken.type == TokenType::RightBrace) break;
            expect(TokenType::Comma);
            if (currentToken.type == TokenType::RightBrace) break;
        }
    }
    expect(TokenType::RightBrace);
    return map;
}

ConfigValue Parser::parseValue() {
    switch (currentToken.type) {
        case TokenType::String: {
            std::string value = currentToken.value;
            nextToken();
            return value;
        }
        case TokenType::Number: {
            std::string value = currentToken.value;
            nextToken();
            if (value.find('.') != std::string::npos) return std::stod(value);
            else return std::stoi(value);
        }
        case TokenType::Boolean: {
            bool value = (currentToken.value == "true");
            nextToken();
            return value;
        }
        case TokenType::LeftBracket: return parseArray();
        case TokenType::LeftParen: return parseSet();
        case TokenType::LeftBrace: return parseMap();
        case TokenType::At: {
            nextToken(); // consume '@'
            if (currentToken.type != TokenType::Identifier) {
                throw std::runtime_error("Expected identifier after '@'");
            }
            std::string macroName = currentToken.value;
            nextToken();
            if (macroMap.count(macroName)) {
                return deep_copy_value(macroMap.at(macroName));
            }
            throw std::runtime_error("Undefined macro: " + macroName);
        }
        case TokenType::Identifier: {
            std::string id = currentToken.value;
            if (id == "List" || id == "list" || id == "Array" || id == "array") {
                nextToken(); // consume identifier
                expect(TokenType::LeftParen);
                auto arr = std::make_unique<Array>();
                if (currentToken.type != TokenType::RightParen) {
                    while (true) {
                        arr->elements.push_back(parseValue());
                        if (currentToken.type == TokenType::RightParen) break;
                        expect(TokenType::Comma);
                        if (currentToken.type == TokenType::RightParen) break; // Trailing comma
                    }
                }
                expect(TokenType::RightParen);
                return arr;
            }
        }
        default: throw std::runtime_error("Unexpected value token: " + currentToken.value);
    }
}

void Parser::resolveInheritance(Config& config) {
    for (auto const& [derivedName, baseNames] : inheritanceMap) {
        if (!config.count(derivedName)) continue;
        ConfigSection mergedSection;
        for (const auto& baseName : baseNames) {
            if (config.count(baseName)) {
                const ConfigSection& baseSection = config.at(baseName);
                for (const auto& [key, value] : baseSection) {
                    mergedSection[key] = deep_copy_value(value);
                }
            }
        }
        const ConfigSection& originalDerivedSection = config.at(derivedName);
        for (const auto& [key, value] : originalDerivedSection) {
            mergedSection[key] = deep_copy_value(value);
        }
        config[derivedName] = std::move(mergedSection);
    }
}

Config Parser::parse() {
    Config config;
    ConfigSection* currentSection = nullptr;
    std::string currentSectionName;

    while (currentToken.type != TokenType::EndOfFile) {
        if (currentToken.type == TokenType::LeftBracket) {
            nextToken(); // Consume '['
            if (currentToken.type == TokenType::Identifier) {
                currentSectionName = currentToken.value;
                currentSection = &config[currentSectionName];
                nextToken(); // Consume identifier
                expect(TokenType::RightBracket);

                // Inheritance logic for regular sections
                if (currentSectionName != "#define" && currentToken.type == TokenType::Colon) {
                    nextToken(); // Consume ':'
                    while (currentToken.type == TokenType::Identifier) {
                        inheritanceMap[currentSectionName].push_back(currentToken.value);
                        nextToken();
                        if (currentToken.type == TokenType::Comma) {
                            nextToken();
                        } else {
                            break;
                        }
                    }
                }
            } else {
                throw std::runtime_error("Unexpected token inside brackets at top level.");
            }
        } else if (currentToken.type == TokenType::Identifier) {
            if (!currentSection) {
                throw std::runtime_error("Key-value pair outside of a section");
            }
            std::string key = currentToken.value;
            nextToken(); // Consume identifier
            expect(TokenType::Equals);

            ConfigValue value = parseValue();

            if (currentSectionName == "#define") {
                macroMap[key] = std::move(value);
            } else {
                (*currentSection)[key] = std::move(value);
            }

        } else if (currentToken.type == TokenType::PlusEquals) {
            if (!currentSection) {
                throw std::runtime_error("Quick registration outside of a section");
            }
            if (currentSectionName == "#define") {
                 throw std::runtime_error("Quick registration is not allowed in [#define] section");
            }
            nextToken(); // Consume '+='
            ConfigValue& registrationList = (*currentSection)[""];
            if (!std::holds_alternative<std::unique_ptr<Array>>(registrationList)) {
                registrationList = std::make_unique<Array>();
            }
            Array* arr = std::get<std::unique_ptr<Array>>(registrationList).get();
            arr->elements.push_back(parseValue());
        }
        else {
            nextToken();
        }
    }

    resolveInheritance(config);

    // Remove the special [#define] section from the final config
    if (config.count("#define")) {
        config.erase("#define");
    }

    return config;
}