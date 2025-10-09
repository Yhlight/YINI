#include "parser.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <filesystem>

// Forward declaration
ConfigValue deep_copy_value(const ConfigValue& val);

// --- Struct Special Member Implementations ---

// Array
Array::Array(const Array& other) {
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
}
Array::~Array() = default;
Array::Array(Array&&) noexcept = default;
Array& Array::operator=(const Array& other) {
    if (this == &other) return *this;
    elements.clear();
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
    return *this;
}
Array& Array::operator=(Array&&) noexcept = default;
bool Array::operator==(const Array& other) const { return elements == other.elements; }

// Set
Set::Set(const Set& other) {
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
}
Set::~Set() = default;
Set::Set(Set&&) noexcept = default;
Set& Set::operator=(const Set& other) {
     if (this == &other) return *this;
    elements.clear();
    elements.reserve(other.elements.size());
    for (const auto& elem : other.elements) {
        elements.push_back(deep_copy_value(elem));
    }
    return *this;
}
Set& Set::operator=(Set&&) noexcept = default;
bool Set::operator==(const Set& other) const { return elements == other.elements; }

// Map
Map::Map(const Map& other) {
    for (const auto& [key, value] : other.elements) {
        elements[key] = deep_copy_value(value);
    }
}
Map::~Map() = default;
Map::Map(Map&&) noexcept = default;
Map& Map::operator=(const Map& other) {
    if (this == &other) return *this;
    elements.clear();
    for (const auto& [key, value] : other.elements) {
        elements[key] = deep_copy_value(value);
    }
    return *this;
}
Map& Map::operator=(Map&&) noexcept = default;
bool Map::operator==(const Map& other) const { return elements == other.elements; }


// DynaValue
DynaValue::DynaValue() = default;
DynaValue::~DynaValue() = default;
DynaValue::DynaValue(DynaValue&& other) noexcept = default;
DynaValue& DynaValue::operator=(DynaValue&& other) noexcept = default;

DynaValue::DynaValue(const DynaValue& other) {
    if (other.value) value = std::make_unique<ConfigValue>(deep_copy_value(*other.value));
    for(const auto& b : other.backup) {
        backup.push_back(std::make_unique<ConfigValue>(deep_copy_value(*b)));
    }
}
DynaValue& DynaValue::operator=(const DynaValue& other) {
    if (this == &other) return *this;
    if (other.value) value = std::make_unique<ConfigValue>(deep_copy_value(*other.value));
    else value.reset();
    backup.clear();
    for(const auto& b : other.backup) {
        backup.push_back(std::make_unique<ConfigValue>(deep_copy_value(*b)));
    }
    return *this;
}
bool DynaValue::operator==(const DynaValue& other) const {
    if ((value && !other.value) || (!value && other.value)) return false;
    if (value && other.value && *value != *other.value) return false;
    if (backup.size() != other.backup.size()) return false;
    for(size_t i = 0; i < backup.size(); ++i) {
        if (*backup[i] != *other.backup[i]) return false;
    }
    return true;
}


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
        } else if constexpr (std::is_same_v<T, std::unique_ptr<DynaValue>>) {
            return v ? std::make_unique<DynaValue>(*v) : nullptr;
        }
        else {
            return v;
        }
    }, val);
}

// --- Arithmetic Helper ---
ConfigValue apply_op(const ConfigValue& left, const ConfigValue& right, const Token& op) {
    if ((std::holds_alternative<int>(left) || std::holds_alternative<double>(left)) &&
        (std::holds_alternative<int>(right) || std::holds_alternative<double>(right)))
    {
        bool is_double = std::holds_alternative<double>(left) || std::holds_alternative<double>(right);
        if (is_double) {
            double val_l = std::holds_alternative<double>(left) ? std::get<double>(left) : static_cast<double>(std::get<int>(left));
            double val_r = std::holds_alternative<double>(right) ? std::get<double>(right) : static_cast<double>(std::get<int>(right));
            switch (op.type) {
                case TokenType::Plus: return val_l + val_r;
                case TokenType::Minus: return val_l - val_r;
                case TokenType::Star: return val_l * val_r;
                case TokenType::Slash: if (val_r == 0) throw ParsingException("Division by zero", op.line, op.column); return val_l / val_r;
                default: throw ParsingException("Invalid operator for doubles", op.line, op.column);
            }
        } else {
            int val_l = std::get<int>(left);
            int val_r = std::get<int>(right);
            switch (op.type) {
                case TokenType::Plus: return val_l + val_r;
                case TokenType::Minus: return val_l - val_r;
                case TokenType::Star: return val_l * val_r;
                case TokenType::Slash: if (val_r == 0) throw ParsingException("Division by zero", op.line, op.column); return val_l / val_r; // Integer division
                case TokenType::Percent: if (val_r == 0) throw ParsingException("Modulo by zero", op.line, op.column); return val_l % val_r;
                default: throw ParsingException("Invalid operator for integers", op.line, op.column);
            }
        }
    }

    throw ParsingException("Invalid operands for arithmetic operation", op.line, op.column);
}


// --- Parser Class Implementations ---
Parser::Parser(Lexer& lexer) : lexer(&lexer) {
    nextToken();
}

void Parser::nextToken() {
    currentToken = lexer->nextToken();
}

void Parser::expect(TokenType type) {
    if (currentToken.type == type) {
        nextToken();
    } else {
        throw ParsingException("Unexpected token", currentToken.line, currentToken.column);
    }
}

SchemaRule Parser::parseSchemaRule() {
    SchemaRule rule;

    auto get_numeric_value = [&](const ConfigValue& val) -> double {
        if (std::holds_alternative<int>(val)) return static_cast<double>(std::get<int>(val));
        if (std::holds_alternative<double>(val)) return std::get<double>(val);
        throw ParsingException("min/max value must be a number", currentToken.line, currentToken.column);
    };

    while (true) {
        if (currentToken.type == TokenType::Bang) {
            rule.required = true;
            nextToken();
        } else if (currentToken.type == TokenType::Question) {
            rule.required = false;
            nextToken();
        } else if (currentToken.type == TokenType::Identifier) {
            const std::string& val = currentToken.value;
            if (val == "~") {
                rule.empty_behavior = '~';
                nextToken();
            } else if (val == "e") {
                rule.empty_behavior = 'e';
                nextToken();
            } else if (val == "min") {
                nextToken();
                expect(TokenType::Equals);
                rule.min_val = get_numeric_value(parse_expression());
            } else if (val == "max") {
                nextToken();
                expect(TokenType::Equals);
                rule.max_val = get_numeric_value(parse_expression());
            } else { // It's a type
                rule.type = val;
                nextToken();
            }
        } else if (currentToken.type == TokenType::Equals) {
            nextToken(); // consume '='
            rule.empty_behavior = '=';
            rule.default_value = parse_expression();
        } else {
            break; // Not a recognized part of a schema rule
        }

        if (currentToken.type == TokenType::Comma) {
            nextToken();
        } else {
            break; // No more parts
        }
    }
    return rule;
}

// --- Expression Parsing ---
ConfigValue Parser::parse_expression() {
    return parse_term();
}

ConfigValue Parser::parse_term() {
    ConfigValue result = parse_factor();
    while (currentToken.type == TokenType::Plus || currentToken.type == TokenType::Minus) {
        Token op = currentToken;
        nextToken();
        ConfigValue right = parse_factor();
        result = apply_op(result, right, op);
    }
    return result;
}

ConfigValue Parser::parse_factor() {
    ConfigValue result = parse_primary();
    while (currentToken.type == TokenType::Star || currentToken.type == TokenType::Slash || currentToken.type == TokenType::Percent) {
        Token op = currentToken;
        nextToken();
        ConfigValue right = parse_primary();
        result = apply_op(result, right, op);
    }
    return result;
}

ConfigValue Parser::parse_primary() {
    Token start_token = currentToken;
    if (currentToken.type == TokenType::Minus) {
        nextToken(); // consume '-'
        ConfigValue operand = parse_primary();
        if (std::holds_alternative<int>(operand)) {
            return -std::get<int>(operand);
        }
        if (std::holds_alternative<double>(operand)) {
            return -std::get<double>(operand);
        }
        throw ParsingException("Unary minus can only be applied to numbers.", start_token.line, start_token.column);
    }
    if (currentToken.type == TokenType::Plus) {
        nextToken(); // consume '+'
        return parse_primary();
    }

    switch (currentToken.type) {
        case TokenType::Number: {
            std::string value = currentToken.value;
            nextToken();
            try {
                if (value.find('.') != std::string::npos) return std::stod(value);
                else return std::stoi(value);
            } catch (const std::invalid_argument& e) {
                throw ParsingException("Invalid number format: " + value, start_token.line, start_token.column);
            }
        }
        case TokenType::String: {
            std::string value = currentToken.value;
            nextToken();
            return value;
        }
        case TokenType::Boolean: {
            bool value = (currentToken.value == "true");
            nextToken();
            return value;
        }
        case TokenType::LeftParen: {
            nextToken(); // consume (
            if (currentToken.type == TokenType::RightParen) {
                nextToken(); // consume ')' for empty set
                return std::make_unique<Set>();
            }

            ConfigValue first_val = parse_expression();

            if (currentToken.type == TokenType::Comma) {
                // It's a Set, not a grouped expression
                auto set = std::make_unique<Set>();
                set->elements.push_back(std::move(first_val));

                while (currentToken.type == TokenType::Comma) {
                    nextToken(); // consume comma
                    if (currentToken.type == TokenType::RightParen) break; // Trailing comma
                    set->elements.push_back(parse_expression());
                }
                expect(TokenType::RightParen);
                return set;
            } else {
                // It's a grouped expression
                expect(TokenType::RightParen);
                return first_val;
            }
        }
        case TokenType::LeftBracket: return parseArray();
        case TokenType::LeftBrace: return parseMap();
        case TokenType::At: {
             nextToken(); // consume '@'
            if (currentToken.type == TokenType::LeftBrace) {
                nextToken();
                if (currentToken.type != TokenType::Identifier) throw ParsingException("Expected section name in reference", currentToken.line, currentToken.column);
                std::string sectionName = currentToken.value;
                nextToken();
                expect(TokenType::Dot);
                if (currentToken.type != TokenType::Identifier) throw ParsingException("Expected key name in reference", currentToken.line, currentToken.column);
                std::string keyName = currentToken.value;
                nextToken();
                expect(TokenType::RightBrace);
                return CrossSectionRef{sectionName, keyName};
            } else {
                if (currentToken.type != TokenType::Identifier) throw ParsingException("Expected identifier after '@'", currentToken.line, currentToken.column);
                std::string macroName = currentToken.value;
                nextToken();
                if (macroMap.count(macroName)) return deep_copy_value(macroMap.at(macroName).value);
                throw ParsingException("Undefined macro: " + macroName, start_token.line, start_token.column);
            }
        }
        case TokenType::Dollar: {
            nextToken(); // consume '$'
            expect(TokenType::LeftBrace);
            if (currentToken.type != TokenType::Identifier) {
                throw ParsingException("Expected environment variable name inside ${...}", currentToken.line, currentToken.column);
            }
            std::string varName = currentToken.value;
            nextToken(); // consume identifier
            expect(TokenType::RightBrace);

            const char* env_val_cstr = getenv(varName.c_str());
            if (!env_val_cstr) {
                throw ParsingException("Environment variable not set: " + varName, start_token.line, start_token.column);
            }
            std::string env_val(env_val_cstr);
            Lexer temp_lexer(env_val);
            Parser temp_parser(temp_lexer);
            return temp_parser.parse_expression();
        }
        case TokenType::Identifier: {
            std::string id = currentToken.value;
            if (id.rfind("#", 0) == 0) { // Starts with #
                nextToken(); // consume identifier
                std::string hex = id.substr(1);
                if (hex.length() != 6) throw ParsingException("Invalid hex color code length: " + hex, start_token.line, start_token.column);
                try {
                    int r = std::stoi(hex.substr(0, 2), nullptr, 16);
                    int g = std::stoi(hex.substr(2, 2), nullptr, 16);
                    int b = std::stoi(hex.substr(4, 2), nullptr, 16);
                    return Color{r, g, b};
                } catch (const std::invalid_argument& e) {
                    throw ParsingException("Invalid hex character in color code: " + hex, start_token.line, start_token.column);
                }
            }
            if (id == "List" || id == "list" || id == "Array" || id == "array") {
                nextToken();
                expect(TokenType::LeftParen);
                auto arr = std::make_unique<Array>();
                if (currentToken.type != TokenType::RightParen) {
                    while (true) {
                        arr->elements.push_back(parse_expression());
                        if (currentToken.type == TokenType::RightParen) break;
                        expect(TokenType::Comma);
                        if (currentToken.type == TokenType::RightParen) break;
                    }
                }
                expect(TokenType::RightParen);
                return arr;
            }
            if (id == "color" || id == "Color") {
                nextToken();
                expect(TokenType::LeftParen);
                int r = std::get<int>(parse_expression());
                expect(TokenType::Comma);
                int g = std::get<int>(parse_expression());
                expect(TokenType::Comma);
                int b = std::get<int>(parse_expression());
                expect(TokenType::RightParen);
                return Color{r, g, b};
            }
            if (id == "Coord" || id == "coord") {
                nextToken();
                expect(TokenType::LeftParen);
                int x = std::get<int>(parse_expression());
                expect(TokenType::Comma);
                int y = std::get<int>(parse_expression());
                int z = 0;
                if (currentToken.type == TokenType::Comma) {
                    nextToken();
                    z = std::get<int>(parse_expression());
                }
                expect(TokenType::RightParen);
                return Coord{x, y, z};
            }
            if (id == "path" || id == "Path") {
                nextToken();
                expect(TokenType::LeftParen);
                if (currentToken.type != TokenType::String) throw ParsingException("path() expects a string literal", currentToken.line, currentToken.column);
                std::string path_val = currentToken.value;
                nextToken();
                expect(TokenType::RightParen);
                return Path{path_val};
            }
             if (id == "Dyna" || id == "dyna") {
                nextToken();
                expect(TokenType::LeftParen);
                auto dyna_val = std::make_unique<DynaValue>();
                dyna_val->value = std::make_unique<ConfigValue>(parse_expression());
                expect(TokenType::RightParen);
                return dyna_val;
            }
        }
        default: throw ParsingException("Unexpected value token: " + currentToken.value, currentToken.line, currentToken.column);
    }
}


// --- Value and Structure Parsing ---
std::unique_ptr<Array> Parser::parseArray() {
    expect(TokenType::LeftBracket);
    auto array = std::make_unique<Array>();
    if (currentToken.type != TokenType::RightBracket) {
        while (true) {
            array->elements.push_back(parse_expression());
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
            set->elements.push_back(parse_expression());
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
            if (currentToken.type != TokenType::Identifier) throw ParsingException("Expected identifier for map key", currentToken.line, currentToken.column);
            std::string key = currentToken.value;
            nextToken();
            expect(TokenType::Colon);
            map->elements[key] = parse_expression();
            if (currentToken.type == TokenType::RightBrace) break;
            expect(TokenType::Comma);
            if (currentToken.type == TokenType::RightBrace) break;
        }
    }
    expect(TokenType::RightBrace);
    return map;
}

// --- Post-Processing ---
void Parser::resolveInheritance(Config& config_ref) {
    for (auto const& [derivedName, baseNames] : inheritanceMap) {
        if (!config_ref.count(derivedName)) continue;
        ConfigSection mergedSection;
        for (const auto& baseName : baseNames) {
            if (config_ref.count(baseName)) {
                const ConfigSection& baseSection = config_ref.at(baseName);
                for (const auto& [key, value] : baseSection) {
                    mergedSection[key] = deep_copy_value(value);
                }
            }
        }
        const ConfigSection& originalDerivedSection = config_ref.at(derivedName);
        for (const auto& [key, value] : originalDerivedSection) {
            mergedSection[key] = deep_copy_value(value);
        }
        config_ref[derivedName] = std::move(mergedSection);
    }
}

void Parser::resolveReferences(Config& config_ref) {
    for (auto& [section_name, section] : config_ref) {
        for (auto& [key, value] : section) {
            if (std::holds_alternative<CrossSectionRef>(value)) {
                const auto& ref = std::get<CrossSectionRef>(value);
                if (config_ref.count(ref.section) && config_ref.at(ref.section).count(ref.key)) {
                    ConfigValue resolved_value = deep_copy_value(config_ref.at(ref.section).at(ref.key));
                    while(std::holds_alternative<CrossSectionRef>(resolved_value)) {
                         const auto& inner_ref = std::get<CrossSectionRef>(resolved_value);
                         if (config_ref.count(inner_ref.section) && config_ref.at(inner_ref.section).count(inner_ref.key)) {
                            resolved_value = deep_copy_value(config_ref.at(inner_ref.section).at(inner_ref.key));
                         } else {
                            throw std::runtime_error("Unresolved cross-section reference: " + inner_ref.section + "." + inner_ref.key);
                         }
                    }
                    value = std::move(resolved_value);
                } else {
                    throw std::runtime_error("Unresolved cross-section reference: " + ref.section + "." + ref.key);
                }
            }
        }
    }
}

void Parser::validate(Config& config) const {
    for (const auto& [section_name, rules] : this->schema) {
        for (const auto& [key, rule] : rules) {
            if (!config.count(section_name) || !config.at(section_name).count(key)) {
                if (rule.required) {
                    if (rule.empty_behavior == 'e') {
                        throw std::runtime_error("Missing required key '" + key + "' in section [" + section_name + "]");
                    } else if (rule.empty_behavior == '=') {
                        if (rule.default_value.has_value()) {
                            config[section_name][key] = deep_copy_value(rule.default_value.value());
                        } else {
                            throw std::runtime_error("Schema error: default value specified for '" + key + "' but no value provided.");
                        }
                    }
                }
            } else {
                auto& value = config.at(section_name).at(key);
                if (rule.type.has_value()) {
                    const auto& type_str = rule.type.value();
                    bool type_ok = false;
                    if (type_str == "int" && std::holds_alternative<int>(value)) type_ok = true;
                    else if (type_str == "float" && std::holds_alternative<double>(value)) type_ok = true;
                    else if (type_str == "string" && std::holds_alternative<std::string>(value)) type_ok = true;
                    else if (type_str == "bool" && std::holds_alternative<bool>(value)) type_ok = true;

                    if (!type_ok) {
                        throw std::runtime_error("Type mismatch for key '" + key + "'. Expected " + type_str);
                    }
                }

                if (std::holds_alternative<int>(value) || std::holds_alternative<double>(value)) {
                    double numeric_value = std::holds_alternative<int>(value) ? static_cast<double>(std::get<int>(value)) : std::get<double>(value);
                    if (rule.min_val.has_value() && numeric_value < rule.min_val.value()) {
                        throw std::runtime_error("Value for key '" + key + "' is below minimum of " + std::to_string(rule.min_val.value()));
                    }
                    if (rule.max_val.has_value() && numeric_value > rule.max_val.value()) {
                        throw std::runtime_error("Value for key '" + key + "' is above maximum of " + std::to_string(rule.max_val.value()));
                    }
                }
            }
        }
    }
}


// --- Main Parsing Logic ---
void Parser::_parse(Lexer& lexer_ref, const std::string& current_dir) {
    this->lexer = &lexer_ref;
    nextToken();

    ConfigSection* currentSection = nullptr;
    std::string currentSectionName;

    while (currentToken.type != TokenType::EndOfFile) {
        if (currentToken.type == TokenType::LeftBracket) {
            nextToken();
            if (currentToken.type == TokenType::Identifier) {
                currentSectionName = currentToken.value;
                if (currentSectionName == "#schema") {
                    in_schema_mode = true;
                    currentSection = nullptr;
                } else if (in_schema_mode) {
                    current_schema_target_section = currentSectionName;
                    currentSection = nullptr;
                } else {
                    in_schema_mode = false;
                    currentSection = &this->config[currentSectionName];
                }
                nextToken();
                expect(TokenType::RightBracket);

                if (!in_schema_mode && currentSectionName != "#define" && currentSectionName != "#include" && currentToken.type == TokenType::Colon) {
                    nextToken();
                    while (currentToken.type == TokenType::Identifier) {
                        inheritanceMap[currentSectionName].push_back(currentToken.value);
                        nextToken();
                        if (currentToken.type == TokenType::Comma) nextToken();
                        else break;
                    }
                }
            } else {
                throw ParsingException("Unexpected token inside brackets at top level.", currentToken.line, currentToken.column);
            }
        } else if (currentToken.type == TokenType::Identifier) {
            if (!currentSection && !in_schema_mode) throw ParsingException("Key-value pair outside of a section", currentToken.line, currentToken.column);

            Token key_token = currentToken;
            std::string key = key_token.value;
            nextToken();
            expect(TokenType::Equals);

            if (in_schema_mode) {
                if (current_schema_target_section.empty()) throw ParsingException("Schema rule defined without a section target", currentToken.line, currentToken.column);
                schema[current_schema_target_section][key] = parseSchemaRule();
            } else {
                ConfigValue value = parse_expression();
                if (currentSectionName == "#define") {
                    macroMap[key] = {std::move(value), key_token.line, key_token.column};
                } else {
                    (*currentSection)[key] = std::move(value);
                }
            }
        } else if (currentToken.type == TokenType::PlusEquals) {
            if (!currentSection) throw ParsingException("Quick registration outside of a section", currentToken.line, currentToken.column);
            nextToken();

            if(currentSectionName == "#include") {
                if (currentToken.type != TokenType::String) {
                    throw ParsingException("Expected quoted filename for include.", currentToken.line, currentToken.column);
                }
                std::string filename = currentToken.value;
                nextToken();

                Lexer* saved_lexer = this->lexer;
                Token saved_token = this->currentToken;

                std::filesystem::path current_path(current_dir);
                std::filesystem::path file_path(filename);
                std::string full_path = (current_path / file_path).string();

                std::ifstream file(full_path);
                if (!file) throw ParsingException("Could not open included file: " + full_path, currentToken.line, currentToken.column);
                std::stringstream buffer;
                buffer << file.rdbuf();
                Lexer included_lexer(buffer.str());

                _parse(included_lexer, (current_path / file_path).parent_path().string());

                this->lexer = saved_lexer;
                this->currentToken = saved_token;
            } else {
                if (currentSectionName == "#define") {
                     throw ParsingException("Quick registration is not allowed in [#define] section", currentToken.line, currentToken.column);
                }
                ConfigValue& registrationList = (*currentSection)[""];
                if (!std::holds_alternative<std::unique_ptr<Array>>(registrationList)) {
                    registrationList = std::make_unique<Array>();
                }
                Array* arr = std::get<std::unique_ptr<Array>>(registrationList).get();
                arr->elements.push_back(parse_expression());
            }
        }
        else if (currentToken.type != TokenType::EndOfFile) {
            throw ParsingException("Unexpected token at top level.", currentToken.line, currentToken.column);
        }
    }
}

Config Parser::parse(const std::string& input) {
    this->config.clear();
    this->macroMap.clear();
    this->inheritanceMap.clear();
    this->schema.clear();
    this->in_schema_mode = false;

    Lexer lexer(input);
    _parse(lexer, ".");

    resolveInheritance(this->config);
    resolveReferences(this->config);

    if (this->config.count("#define")) {
        this->config.erase("#define");
    }
    if (this->config.count("#schema")) {
        this->config.erase("#schema");
    }
    return std::move(this->config);
}

Config Parser::parseFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    this->config.clear();
    this->macroMap.clear();
    this->inheritanceMap.clear();
    this->schema.clear();
    this->in_schema_mode = false;

    std::filesystem::path fs_path(filepath);
    Lexer lexer(buffer.str());
    _parse(lexer, fs_path.parent_path().string());

    resolveInheritance(this->config);
    resolveReferences(this->config);

    if (this->config.count("#define")) {
        this->config.erase("#define");
    }
    if (this->config.count("#schema")) {
        this->config.erase("#schema");
    }

    return std::move(this->config);
}

ConfigValue Parser::parseValue(const std::string& input) {
    Lexer temp_lexer(input);
    this->lexer = &temp_lexer;
    nextToken();
    return parse_expression();
}

const Schema& Parser::getSchema() const {
    return schema;
}

std::map<std::string, Parser::MacroDefinition> Parser::getMacroMap() {
    return std::move(macroMap);
}

// --- JSON Serialization Implementations ---
void to_json(nlohmann::json& j, const ConfigValue& val) {
    std::visit([&j](const auto& v) -> void {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
            if (v) j = *v; else j = nullptr;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Set>>) {
            if (v) j = *v; else j = nullptr;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Map>>) {
            if (v) j = *v; else j = nullptr;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<DynaValue>>) {
            if (v) j = *v; else j = nullptr;
        }
        else {
            j = v;
        }
    }, val);
}

void to_json(nlohmann::json& j, const Array& a) {
    j = nlohmann::json::array();
    for (const auto& elem : a.elements) {
        j.push_back(elem);
    }
}

void to_json(nlohmann::json& j, const Set& s) {
    j = nlohmann::json::array();
    for (const auto& elem : s.elements) {
        j.push_back(elem);
    }
}

void to_json(nlohmann::json& j, const Map& m) {
    j = nlohmann::json::object();
    for (const auto& [key, value] : m.elements) {
        j[key] = value;
    }
}

void to_json(nlohmann::json& j, const CrossSectionRef& r) {
    j = nlohmann::json{{"__type", "CrossSectionRef"}, {"section", r.section}, {"key", r.key}};
}

void to_json(nlohmann::json& j, const Color& c) {
    j = nlohmann::json{{"__type", "Color"}, {"r", c.r}, {"g", c.g}, {"b", c.b}};
}

void to_json(nlohmann::json& j, const Coord& c) {
    j = nlohmann::json{{"__type", "Coord"}, {"x", c.x}, {"y", c.y}, {"z", c.z}};
}

void to_json(nlohmann::json& j, const Path& p) {
    j = nlohmann::json{{"__type", "Path"}, {"value", p.value}};
}

void to_json(nlohmann::json& j, const DynaValue& d) {
    nlohmann::json val_json = nullptr;
    if (d.value) {
        to_json(val_json, *d.value);
    }

    nlohmann::json backup_json = nlohmann::json::array();
    for (const auto& b : d.backup) {
        nlohmann::json b_json;
        to_json(b_json, *b);
        backup_json.push_back(b_json);
    }

    j = nlohmann::json{
        {"__type", "DynaValue"},
        {"value", val_json},
        {"backup", backup_json}
    };
}

// --- JSON Deserialization Implementations ---
void from_json(const nlohmann::json& j, DynaValue& d) {
    if (j.contains("__type") && j["__type"] == "DynaValue") {
        if (j.contains("value") && !j["value"].is_null()) {
            d.value = std::make_unique<ConfigValue>(j["value"].get<ConfigValue>());
        }
        if (j.contains("backup") && j["backup"].is_array()) {
            for (const auto& backup_item : j["backup"]) {
                d.backup.push_back(std::make_unique<ConfigValue>(backup_item.get<ConfigValue>()));
            }
        }
    }
}

void from_json(const nlohmann::json& j, Path& p) {
    if (j.contains("__type") && j["__type"] == "Path") {
        p.value = j.at("value").get<std::string>();
    }
}

void from_json(const nlohmann::json& j, Coord& c) {
    if (j.contains("__type") && j["__type"] == "Coord") {
        c.x = j.at("x").get<int>();
        c.y = j.at("y").get<int>();
        c.z = j.at("z").get<int>();
    }
}

void from_json(const nlohmann::json& j, Color& c) {
    if (j.contains("__type") && j["__type"] == "Color") {
        c.r = j.at("r").get<int>();
        c.g = j.at("g").get<int>();
        c.b = j.at("b").get<int>();
    }
}

void from_json(const nlohmann::json& j, CrossSectionRef& r) {
    if (j.contains("__type") && j["__type"] == "CrossSectionRef") {
        r.section = j.at("section").get<std::string>();
        r.key = j.at("key").get<std::string>();
    }
}

void from_json(const nlohmann::json& j, Map& m) {
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            m.elements[it.key()] = it.value().get<ConfigValue>();
        }
    }
}

void from_json(const nlohmann::json& j, Set& s) {
    if (j.is_array()) {
        for (const auto& item : j) {
            s.elements.push_back(item.get<ConfigValue>());
        }
    }
}

void from_json(const nlohmann::json& j, Array& a) {
    if (j.is_array()) {
        for (const auto& item : j) {
            a.elements.push_back(item.get<ConfigValue>());
        }
    }
}

void from_json(const nlohmann::json& j, ConfigValue& val) {
    if (j.is_string()) {
        val = j.get<std::string>();
    } else if (j.is_number_integer()) {
        val = j.get<int>();
    } else if (j.is_number_float()) {
        val = j.get<double>();
    } else if (j.is_boolean()) {
        val = j.get<bool>();
    } else if (j.is_array()) {
        val = std::make_unique<Array>(j.get<Array>());
    } else if (j.is_object()) {
        if (j.contains("__type")) {
            const std::string type = j["__type"];
            if (type == "CrossSectionRef") val = j.get<CrossSectionRef>();
            else if (type == "Color") val = j.get<Color>();
            else if (type == "Coord") val = j.get<Coord>();
            else if (type == "Path") val = j.get<Path>();
            else if (type == "DynaValue") val = std::make_unique<DynaValue>(j.get<DynaValue>());
            else val = std::make_unique<Map>(j.get<Map>()); // Default to Map if type is unknown
        } else {
            val = std::make_unique<Map>(j.get<Map>());
        }
    } else if (j.is_null()) {
        // How to handle null? For now, maybe an empty string or a special null type if added later.
        val = std::string();
    }
}

// --- YINI String Serialization ---
std::string to_yini_string(const ConfigValue& val) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        std::stringstream ss;

        if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + v + "\"";
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
            return std::to_string(v);
        } else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Array>>) {
            ss << "[";
            for (size_t i = 0; i < v->elements.size(); ++i) {
                ss << to_yini_string(v->elements[i]) << (i == v->elements.size() - 1 ? "" : ", ");
            }
            ss << "]";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Set>>) {
            ss << "(";
            for (size_t i = 0; i < v->elements.size(); ++i) {
                ss << to_yini_string(v->elements[i]) << (i == v->elements.size() - 1 ? "" : ", ");
            }
            ss << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::unique_ptr<Map>>) {
            ss << "{";
            size_t i = 0;
            for (const auto& [key, value] : v->elements) {
                ss << key << ": " << to_yini_string(value) << (i++ == v->elements.size() - 1 ? "" : ", ");
            }
            ss << "}";
            return ss.str();
        } else if constexpr (std::is_same_v<T, CrossSectionRef>) {
            return "@{" + v.section + "." + v.key + "}";
        } else if constexpr (std::is_same_v<T, Color>) {
            return "color(" + std::to_string(v.r) + ", " + std::to_string(v.g) + ", " + std::to_string(v.b) + ")";
        } else if constexpr (std::is_same_v<T, Coord>) {
            return "Coord(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        } else if constexpr (std::is_same_v<T, Path>) {
            return "path(\"" + v.value + "\")";
        } else if constexpr (std::is_same_v<T, std::unique_ptr<DynaValue>>) {
            if (v && v->value) {
                return "Dyna(" + to_yini_string(*v->value) + ")";
            }
            return "Dyna(null)";
        }
        return "null"; // Should not happen for valid ConfigValue
    }, val);
}