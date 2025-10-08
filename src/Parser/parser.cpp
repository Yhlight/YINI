#include "parser.h"
#include <stdexcept>

// Forward declaration for the recursive deep copy function
ConfigValue deep_copy_value(const ConfigValue& val);

// --- Array Struct Implementations ---

// Custom copy constructor to perform a deep copy of elements
Array::Array(const Array& other) {
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
}

// Equality operator for comparing arrays, essential for testing
bool Array::operator==(const Array& other) const {
    if (elements.size() != other.elements.size()) {
        return false;
    }
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i].index() != other.elements[i].index()) {
            return false;
        }
        if (std::holds_alternative<std::unique_ptr<Array>>(elements[i])) {
            if (!(*std::get<std::unique_ptr<Array>>(elements[i]) == *std::get<std::unique_ptr<Array>>(other.elements[i]))) {
                return false;
            }
        } else if (elements[i] != other.elements[i]) {
            return false;
        }
    }
    return true;
}

// --- Deep Copy Helper ---

// Helper function to deep-copy a ConfigValue variant
ConfigValue deep_copy_value(const ConfigValue& val) {
    return std::visit([](const auto& v) -> ConfigValue {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
            if (v) {
                // Use the Array's deep-copy constructor
                return std::make_unique<Array>(*v);
            } else {
                return std::unique_ptr<Array>(nullptr);
            }
        } else {
            // Other types (int, string, etc.) are copyable by value
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

    if (currentToken.type == TokenType::RightBracket) {
        nextToken();
        return array;
    }

    while (true) {
        array->elements.push_back(parseValue());

        if (currentToken.type == TokenType::RightBracket) {
            nextToken();
            break;
        }
        expect(TokenType::Comma);
        if (currentToken.type == TokenType::RightBracket) {
            nextToken();
            break;
        }
    }
    return array;
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
            if (value.find('.') != std::string::npos) {
                return std::stod(value);
            } else {
                return std::stoi(value);
            }
        }
        case TokenType::Boolean: {
            bool value = (currentToken.value == "true");
            nextToken();
            return value;
        }
        case TokenType::LeftBracket:
            return parseArray();
        default:
            throw std::runtime_error("Unexpected value token");
    }
}

void Parser::resolveInheritance(Config& config) {
    for (auto const& [derivedName, baseNames] : inheritanceMap) {
        if (!config.count(derivedName)) continue;

        ConfigSection mergedSection;

        // 1. Process base sections in order.
        for (const auto& baseName : baseNames) {
            if (config.count(baseName)) {
                const ConfigSection& baseSection = config.at(baseName);
                for (const auto& [key, value] : baseSection) {
                    mergedSection[key] = deep_copy_value(value); // Assign, allowing override
                }
            }
        }

        // 2. Process the derived section's own keys, which have top priority.
        const ConfigSection& originalDerivedSection = config.at(derivedName);
        for (const auto& [key, value] : originalDerivedSection) {
            mergedSection[key] = deep_copy_value(value); // Assign, allowing override
        }

        // 3. Replace the derived section in the main config.
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

                if (currentToken.type == TokenType::Colon) {
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
            nextToken();
            expect(TokenType::Equals);
            (*currentSection)[key] = parseValue();
        } else if (currentToken.type == TokenType::PlusEquals) {
            if (!currentSection) {
                throw std::runtime_error("Quick registration outside of a section");
            }
            nextToken(); // Consume '+='

            // Find or create the implicit array under the empty key ""
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

    return config;
}