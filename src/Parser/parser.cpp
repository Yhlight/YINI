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
ConfigValue apply_op(const ConfigValue& left, const ConfigValue& right, TokenType op) {
    if ((std::holds_alternative<int>(left) || std::holds_alternative<double>(left)) &&
        (std::holds_alternative<int>(right) || std::holds_alternative<double>(right)))
    {
        bool is_double = std::holds_alternative<double>(left) || std::holds_alternative<double>(right);
        if (is_double) {
            double val_l = std::holds_alternative<double>(left) ? std::get<double>(left) : static_cast<double>(std::get<int>(left));
            double val_r = std::holds_alternative<double>(right) ? std::get<double>(right) : static_cast<double>(std::get<int>(right));
            switch (op) {
                case TokenType::Plus: return val_l + val_r;
                case TokenType::Minus: return val_l - val_r;
                case TokenType::Star: return val_l * val_r;
                case TokenType::Slash: if (val_r == 0) throw std::runtime_error("Division by zero"); return val_l / val_r;
                default: throw std::runtime_error("Invalid operator for doubles");
            }
        } else {
            int val_l = std::get<int>(left);
            int val_r = std::get<int>(right);
            switch (op) {
                case TokenType::Plus: return val_l + val_r;
                case TokenType::Minus: return val_l - val_r;
                case TokenType::Star: return val_l * val_r;
                case TokenType::Slash: if (val_r == 0) throw std::runtime_error("Division by zero"); return val_l / val_r; // Integer division
                case TokenType::Percent: if (val_r == 0) throw std::runtime_error("Modulo by zero"); return val_l % val_r;
                default: throw std::runtime_error("Invalid operator for integers");
            }
        }
    }

    throw std::runtime_error("Invalid operands for arithmetic operation");
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
        throw std::runtime_error("Unexpected token");
    }
}

SchemaRule Parser::parseSchemaRule() {
    SchemaRule rule;

    auto get_numeric_value = [](const ConfigValue& val) -> double {
        if (std::holds_alternative<int>(val)) return static_cast<double>(std::get<int>(val));
        if (std::holds_alternative<double>(val)) return std::get<double>(val);
        throw std::runtime_error("min/max value must be a number");
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
        result = apply_op(result, right, op.type);
    }
    return result;
}

ConfigValue Parser::parse_factor() {
    ConfigValue result = parse_primary();
    while (currentToken.type == TokenType::Star || currentToken.type == TokenType::Slash || currentToken.type == TokenType::Percent) {
        Token op = currentToken;
        nextToken();
        ConfigValue right = parse_primary();
        result = apply_op(result, right, op.type);
    }
    return result;
}

ConfigValue Parser::parse_primary() {
    if (currentToken.type == TokenType::Minus) {
        nextToken(); // consume '-'
        ConfigValue operand = parse_primary();
        if (std::holds_alternative<int>(operand)) {
            return -std::get<int>(operand);
        }
        if (std::holds_alternative<double>(operand)) {
            return -std::get<double>(operand);
        }
        throw std::runtime_error("Unary minus can only be applied to numbers.");
    }
    if (currentToken.type == TokenType::Plus) {
        nextToken(); // consume '+'
        return parse_primary();
    }

    switch (currentToken.type) {
        case TokenType::Number: {
            std::string value = currentToken.value;
            nextToken();
            if (value.find('.') != std::string::npos) return std::stod(value);
            else return std::stoi(value);
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
                if (currentToken.type != TokenType::Identifier) throw std::runtime_error("Expected section name in reference");
                std::string sectionName = currentToken.value;
                nextToken();
                expect(TokenType::Dot);
                if (currentToken.type != TokenType::Identifier) throw std::runtime_error("Expected key name in reference");
                std::string keyName = currentToken.value;
                nextToken();
                expect(TokenType::RightBrace);
                return CrossSectionRef{sectionName, keyName};
            } else {
                if (currentToken.type != TokenType::Identifier) throw std::runtime_error("Expected identifier after '@'");
                std::string macroName = currentToken.value;
                nextToken();
                if (macroMap.count(macroName)) return deep_copy_value(macroMap.at(macroName));
                throw std::runtime_error("Undefined macro: " + macroName);
            }
        }
        case TokenType::Dollar: {
            nextToken(); // consume '$'
            expect(TokenType::LeftBrace);
            if (currentToken.type != TokenType::Identifier) {
                throw std::runtime_error("Expected environment variable name inside ${...}");
            }
            std::string varName = currentToken.value;
            nextToken(); // consume identifier
            expect(TokenType::RightBrace);

            const char* env_val_cstr = getenv(varName.c_str());
            if (!env_val_cstr) {
                throw std::runtime_error("Environment variable not set: " + varName);
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
                if (hex.length() != 6) throw std::runtime_error("Invalid hex color code length: " + hex);
                try {
                    int r = std::stoi(hex.substr(0, 2), nullptr, 16);
                    int g = std::stoi(hex.substr(2, 2), nullptr, 16);
                    int b = std::stoi(hex.substr(4, 2), nullptr, 16);
                    return Color{r, g, b};
                } catch (const std::invalid_argument& e) {
                    throw std::runtime_error("Invalid hex character in color code: " + hex);
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
                if (currentToken.type != TokenType::String) throw std::runtime_error("path() expects a string literal");
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
        default: throw std::runtime_error("Unexpected value token: " + currentToken.value);
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
            if (currentToken.type != TokenType::Identifier) throw std::runtime_error("Expected identifier for map key");
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
                throw std::runtime_error("Unexpected token inside brackets at top level.");
            }
        } else if (currentToken.type == TokenType::Identifier) {
            if (!currentSection && !in_schema_mode) throw std::runtime_error("Key-value pair outside of a section");

            std::string key = currentToken.value;
            nextToken();
            expect(TokenType::Equals);

            if (in_schema_mode) {
                if (current_schema_target_section.empty()) throw std::runtime_error("Schema rule defined without a section target");
                schema[current_schema_target_section][key] = parseSchemaRule();
            } else {
                ConfigValue value = parse_expression();
                if (currentSectionName == "#define") {
                    macroMap[key] = std::move(value);
                } else {
                    (*currentSection)[key] = std::move(value);
                }
            }
        } else if (currentToken.type == TokenType::PlusEquals) {
            if (!currentSection) throw std::runtime_error("Quick registration outside of a section");
            nextToken();

            if(currentSectionName == "#include") {
                if (currentToken.type != TokenType::String) {
                    throw std::runtime_error("Expected quoted filename for include.");
                }
                std::string filename = currentToken.value;
                nextToken();

                Lexer* saved_lexer = this->lexer;
                Token saved_token = this->currentToken;

                std::filesystem::path current_path(current_dir);
                std::filesystem::path file_path(filename);
                std::string full_path = (current_path / file_path).string();

                std::ifstream file(full_path);
                if (!file) throw std::runtime_error("Could not open included file: " + full_path);
                std::stringstream buffer;
                buffer << file.rdbuf();
                Lexer included_lexer(buffer.str());

                _parse(included_lexer, (current_path / file_path).parent_path().string());

                this->lexer = saved_lexer;
                this->currentToken = saved_token;
            } else {
                if (currentSectionName == "#define") {
                     throw std::runtime_error("Quick registration is not allowed in [#define] section");
                }
                ConfigValue& registrationList = (*currentSection)[""];
                if (!std::holds_alternative<std::unique_ptr<Array>>(registrationList)) {
                    registrationList = std::make_unique<Array>();
                }
                Array* arr = std::get<std::unique_ptr<Array>>(registrationList).get();
                arr->elements.push_back(parse_expression());
            }
        }
        else {
            nextToken();
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

const Schema& Parser::getSchema() const {
    return schema;
}