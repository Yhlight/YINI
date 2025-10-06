#include "Parser.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace yini
{

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens)
    , current(0)
    , quick_register_counter(0)
    , last_error("")
{
}

Parser::Parser(const std::string& source)
    : current(0)
    , quick_register_counter(0)
    , last_error("")
{
    Lexer lexer(source);
    tokens = lexer.tokenize();
    
    if (lexer.hasError())
    {
        last_error = lexer.getLastError();
    }
}

bool Parser::parse()
{
    while (!isAtEnd())
    {

        // Skip newlines
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        
        // Check for special sections
        if (check(TokenType::LBRACKET))
        {
            // Make sure we don't go out of bounds
            if (current + 1 >= tokens.size())
            {
                error("Unexpected end of file");
                return false;
            }
            
            Token next = tokens[current + 1];
            
            if (next.type == TokenType::HASH)
            {
                advance(); // [
                advance(); // #
                
                Token directive = advance();
                if (directive.type == TokenType::IDENTIFIER)
                {
                    std::string dir_name = directive.getValue<std::string>();
                    
                    if (dir_name == "define")
                    {
                        if (!parseDefineSection())
                        {
                            return false;
                        }
                    }
                    else if (dir_name == "include")
                    {
                        if (!parseIncludeSection())
                        {
                            return false;
                        }
                    }
                    else if (dir_name == "schema")
                    {
                        if (!parseSchemaSection())
                        {
                            return false;
                        }
                    }
                    else
                    {
                        error("Unknown directive: " + dir_name);
                        return false;
                    }
                }
                else
                {
                    error("Expected directive name after [#");
                    return false;
                }
            }
            else
            {
                if (!parseSection())
                {
                    return false;
                }
            }
        }
        else
        {
            error("Unexpected token at top level");
            return false;
        }
    }
    
    // Resolve inheritance
    resolveInheritance();
    
    // Validate against schema
    if (!schema.empty())
    {
        if (!validateAgainstSchema())
        {
            return false;
        }
    }
    
    return true;
}

bool Parser::parseSection()
{
    if (!match(TokenType::LBRACKET))
    {
        error("Expected '[' at start of section");
        return false;
    }
    
    Token name_token = advance();
    if (name_token.type != TokenType::IDENTIFIER)
    {
        error("Expected section name");
        return false;
    }
    
    std::string section_name = name_token.getValue<std::string>();
    Section section(section_name);
    
    if (!match(TokenType::RBRACKET))
    {
        error("Expected ']' after section name");
        return false;
    }
    
    // Check for inheritance
    if (match(TokenType::COLON))
    {
        // Parse inherited sections
        do
        {
            Token inherited = advance();
            if (inherited.type != TokenType::IDENTIFIER)
            {
                error("Expected section name in inheritance list");
                return false;
            }
            
            section.inherited_sections.push_back(inherited.getValue<std::string>());
        }
        while (match(TokenType::COMMA));
    }
    
    // Skip newlines
    while (match(TokenType::NEWLINE))
    {
    }
    
    // Parse entries
    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        // Skip newlines
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        
        // Check for quick register
        if (check(TokenType::PLUS_EQUALS))
        {
            if (!parseQuickRegister(section))
            {
                return false;
            }
        }
        // Otherwise parse key-value pair
        else if (check(TokenType::IDENTIFIER))
        {
            if (!parseKeyValuePair(section))
            {
                return false;
            }
        }
        else if (check(TokenType::LBRACKET))
        {
            // Start of next section
            break;
        }
        else
        {
            error("Expected key or += in section");
            return false;
        }
    }
    
    sections[section_name] = section;
    return true;
}

bool Parser::parseDefineSection()
{
    if (!match(TokenType::RBRACKET))
    {
        error("Expected ']' after [#define");
        return false;
    }
    
    while (match(TokenType::NEWLINE))
    {
    }
    
    // Parse definitions
    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        
        Token name = advance();
        if (name.type != TokenType::IDENTIFIER)
        {
            error("Expected identifier in define section");
            return false;
        }
        
        if (!match(TokenType::EQUALS))
        {
            error("Expected '=' in define section");
            return false;
        }
        
        auto value = parseValue();
        if (!value)
        {
            return false;
        }
        
        defines[name.getValue<std::string>()] = value;
        
        while (match(TokenType::NEWLINE))
        {
        }
    }
    
    return true;
}

bool Parser::parseIncludeSection()
{
    if (!match(TokenType::RBRACKET))
    {
        error("Expected ']' after [#include");
        return false;
    }
    
    while (match(TokenType::NEWLINE))
    {
    }
    
    // Parse include entries
    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        
        if (!match(TokenType::PLUS_EQUALS))
        {
            error("Expected '+=' in include section");
            return false;
        }
        
        Token path = advance();
        if (path.type != TokenType::STRING && path.type != TokenType::IDENTIFIER)
        {
            error("Expected path in include section");
            return false;
        }
        
        std::string include_path = path.getValue<std::string>();
        includes.push_back(include_path);
        
        while (match(TokenType::NEWLINE))
        {
        }
    }
    
    return true;
}

bool Parser::parseSchemaSection()
{
    if (!match(TokenType::RBRACKET))
    {
        error("Expected ']' after [#schema");
        return false;
    }
    
    while (match(TokenType::NEWLINE))
    {
    }
    
    // Parse schema definitions
    while (!isAtEnd() && !check(TokenType::LBRACKET) && !check(TokenType::HASH))
    {
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        
        // Should be a section header
        if (!match(TokenType::LBRACKET))
        {
            error("Expected section header in schema");
            return false;
        }
        
        Token section_name = advance();
        if (section_name.type != TokenType::IDENTIFIER)
        {
            error("Expected section name in schema");
            return false;
        }
        
        if (!match(TokenType::RBRACKET))
        {
            error("Expected ']' after section name in schema");
            return false;
        }
        
        std::string sec_name = section_name.getValue<std::string>();
        std::map<std::string, SchemaRule> rules;
        
        while (match(TokenType::NEWLINE))
        {
        }
        
        // Parse rules for this section
        while (!isAtEnd() && !check(TokenType::LBRACKET))
        {
            if (match(TokenType::NEWLINE))
            {
                continue;
            }
            
            Token key = advance();
            if (key.type != TokenType::IDENTIFIER)
            {
                break; // End of this section's rules
            }
            
            if (!match(TokenType::EQUALS))
            {
                error("Expected '=' in schema rule");
                return false;
            }
            
            SchemaRule rule;
            rule.required = false;
            rule.null_behavior = SchemaRule::NullBehavior::IGNORE;
            
            // Parse rule components: !, int, =1280
            // Format: [!|?], [type], [~|=value|e]
            
            std::string key_name = key.getValue<std::string>();
            
            while (!isAtEnd() && !check(TokenType::NEWLINE) && !check(TokenType::LBRACKET))
            {
                Token component = advance();
                
                // Check for required/optional marker
                if (component.type == TokenType::EXCLAMATION)
                {
                    rule.required = true;
                }
                else if (component.type == TokenType::QUESTION)
                {
                    rule.required = false;
                }
                // Check for type
                else if (component.type == TokenType::IDENTIFIER)
                {
                    std::string type_str = component.getValue<std::string>();
                    if (type_str == "int") rule.value_type = ValueType::INTEGER;
                    else if (type_str == "float") rule.value_type = ValueType::FLOAT;
                    else if (type_str == "bool") rule.value_type = ValueType::BOOLEAN;
                    else if (type_str == "string") rule.value_type = ValueType::STRING;
                    else if (type_str == "array") rule.value_type = ValueType::ARRAY;
                    else if (type_str == "list") rule.value_type = ValueType::LIST;
                    else if (type_str == "map") rule.value_type = ValueType::MAP;
                    else if (type_str == "color") rule.value_type = ValueType::COLOR;
                    else if (type_str == "coord") rule.value_type = ValueType::COORD;
                    else if (type_str == "path") rule.value_type = ValueType::PATH;
                    else if (type_str == "e")
                    {
                        // Error on null
                        rule.null_behavior = SchemaRule::NullBehavior::ERROR;
                    }
                }
                // Check for tilde (ignore null)
                else if (component.type == TokenType::IDENTIFIER && component.getValue<std::string>() == "~")
                {
                    rule.null_behavior = SchemaRule::NullBehavior::IGNORE;
                }
                // Check for equals (default value)
                else if (component.type == TokenType::EQUALS)
                {
                    rule.null_behavior = SchemaRule::NullBehavior::DEFAULT;
                    // Parse default value
                    auto default_val = parseValue();
                    if (default_val)
                    {
                        rule.default_value = default_val;
                    }
                }
                // Skip commas
                else if (component.type == TokenType::COMMA)
                {
                    continue;
                }
            }
            
            rules[key_name] = rule;
            
            while (match(TokenType::NEWLINE))
            {
            }
        }
        
        schema[sec_name] = rules;
    }
    
    return true;
}

bool Parser::parseKeyValuePair(Section& section)
{
    Token key = advance();
    if (key.type != TokenType::IDENTIFIER)
    {
        error("Expected key identifier");
        return false;
    }
    
    if (!match(TokenType::EQUALS))
    {
        error("Expected '=' after key");
        return false;
    }
    
    auto value = parseValue();
    if (!value)
    {
        return false;
    }
    
    section.entries[key.getValue<std::string>()] = value;
    return true;
}

bool Parser::parseQuickRegister(Section& section)
{
    if (!match(TokenType::PLUS_EQUALS))
    {
        error("Expected '+='");
        return false;
    }
    
    auto value = parseValue();
    if (!value)
    {
        return false;
    }
    
    std::string key = std::to_string(quick_register_counter++);
    section.entries[key] = value;
    return true;
}

std::shared_ptr<Value> Parser::parseValue()
{
    return parseExpression();
}

std::shared_ptr<Value> Parser::parseExpression()
{
    auto left = parseTerm();
    if (!left)
    {
        return nullptr;
    }
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS))
    {
        TokenType op = tokens[current - 1].type;
        auto right = parseTerm();
        if (!right)
        {
            return nullptr;
        }
        
        // Perform arithmetic
        if (left->isInteger() && right->isInteger())
        {
            int64_t result = (op == TokenType::PLUS) 
                ? (left->asInteger() + right->asInteger())
                : (left->asInteger() - right->asInteger());
            left = std::make_shared<Value>(result);
        }
        else if ((left->isFloat() || left->isInteger()) && 
                 (right->isFloat() || right->isInteger()))
        {
            double l_val = left->isFloat() ? left->asFloat() : left->asInteger();
            double r_val = right->isFloat() ? right->asFloat() : right->asInteger();
            double result = (op == TokenType::PLUS) ? (l_val + r_val) : (l_val - r_val);
            left = std::make_shared<Value>(result);
        }
        else
        {
            error("Cannot perform arithmetic on non-numeric values");
            return nullptr;
        }
    }
    
    return left;
}

std::shared_ptr<Value> Parser::parseTerm()
{
    auto left = parseFactor();
    if (!left)
    {
        return nullptr;
    }
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO))
    {
        TokenType op = tokens[current - 1].type;
        auto right = parseFactor();
        if (!right)
        {
            return nullptr;
        }
        
        // Perform arithmetic
        if (left->isInteger() && right->isInteger())
        {
            int64_t result;
            if (op == TokenType::MULTIPLY)
            {
                result = left->asInteger() * right->asInteger();
            }
            else if (op == TokenType::DIVIDE)
            {
                result = left->asInteger() / right->asInteger();
            }
            else
            {
                result = left->asInteger() % right->asInteger();
            }
            left = std::make_shared<Value>(result);
        }
        else if ((left->isFloat() || left->isInteger()) && 
                 (right->isFloat() || right->isInteger()))
        {
            double l_val = left->isFloat() ? left->asFloat() : left->asInteger();
            double r_val = right->isFloat() ? right->asFloat() : right->asInteger();
            double result;
            if (op == TokenType::MULTIPLY)
            {
                result = l_val * r_val;
            }
            else
            {
                result = l_val / r_val;
            }
            left = std::make_shared<Value>(result);
        }
        else
        {
            error("Cannot perform arithmetic on non-numeric values");
            return nullptr;
        }
    }
    
    return left;
}

std::shared_ptr<Value> Parser::parseFactor()
{
    // Unary minus
    if (match(TokenType::MINUS))
    {
        auto value = parseFactor();
        if (!value)
        {
            return nullptr;
        }
        
        if (value->isInteger())
        {
            return std::make_shared<Value>(-value->asInteger());
        }
        else if (value->isFloat())
        {
            return std::make_shared<Value>(-value->asFloat());
        }
        else
        {
            error("Cannot negate non-numeric value");
            return nullptr;
        }
    }
    
    // Parentheses
    if (match(TokenType::LPAREN))
    {
        auto value = parseExpression();
        if (!match(TokenType::RPAREN))
        {
            error("Expected ')' after expression");
            return nullptr;
        }
        return value;
    }
    
    return parsePrimary();
}

std::shared_ptr<Value> Parser::parsePrimary()
{
    Token token = peek();
    
    // Literals
    if (token.type == TokenType::INTEGER)
    {
        advance();
        return std::make_shared<Value>(token.getValue<int64_t>());
    }
    
    if (token.type == TokenType::FLOAT)
    {
        advance();
        return std::make_shared<Value>(token.getValue<double>());
    }
    
    if (token.type == TokenType::BOOLEAN)
    {
        advance();
        return std::make_shared<Value>(token.getValue<bool>());
    }
    
    if (token.type == TokenType::STRING)
    {
        advance();
        return std::make_shared<Value>(token.getValue<std::string>());
    }
    
    // Arrays
    if (token.type == TokenType::LBRACKET)
    {
        return parseArray();
    }
    
    // Maps/tuples
    if (token.type == TokenType::LBRACE)
    {
        return parseMap();
    }
    
    // Sets
    if (token.type == TokenType::LPAREN)
    {
        return parseSet();
    }
    
    // Hex color or Color() constructor
    if (token.type == TokenType::COLOR)
    {
        return parseColor();
    }
    
    if (token.type == TokenType::COORD)
    {
        return parseCoord();
    }
    
    if (token.type == TokenType::PATH)
    {
        return parsePath();
    }
    
    if (token.type == TokenType::LIST)
    {
        return parseList();
    }
    
    if (token.type == TokenType::ARRAY)
    {
        advance(); // Array keyword
        if (!match(TokenType::LPAREN))
        {
            error("Expected '(' after Array");
            return nullptr;
        }
        auto arr = parseArray();
        // Convert to array type if needed
        return arr;
    }
    
    if (token.type == TokenType::DYNA)
    {
        return parseDynamic();
    }
    
    // References
    if (token.type == TokenType::AT)
    {
        return parseReference();
    }
    
    if (token.type == TokenType::AT_LBRACE)
    {
        return parseReference();
    }
    
    // Environment variables
    if (token.type == TokenType::DOLLAR_LBRACE)
    {
        return parseEnvVar();
    }
    
    error("Unexpected token in value: " + token.toString());
    return nullptr;
}

std::shared_ptr<Value> Parser::parseArray()
{
    if (!match(TokenType::LBRACKET))
    {
        error("Expected '[' at start of array");
        return nullptr;
    }
    
    Value::ArrayType elements;
    
    while (!check(TokenType::RBRACKET) && !isAtEnd())
    {
        auto element = parseValue();
        if (!element)
        {
            return nullptr;
        }
        
        elements.push_back(element);
        
        if (!match(TokenType::COMMA))
        {
            break;
        }
    }
    
    if (!match(TokenType::RBRACKET))
    {
        error("Expected ']' at end of array");
        return nullptr;
    }
    
    return std::make_shared<Value>(elements);
}

std::shared_ptr<Value> Parser::parseList()
{
    advance(); // List keyword
    
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' after List");
        return nullptr;
    }
    
    Value::ArrayType elements;
    
    while (!check(TokenType::RPAREN) && !isAtEnd())
    {
        auto element = parseValue();
        if (!element)
        {
            return nullptr;
        }
        
        elements.push_back(element);
        
        if (!match(TokenType::COMMA))
        {
            break;
        }
    }
    
    if (!match(TokenType::RPAREN))
    {
        error("Expected ')' at end of list");
        return nullptr;
    }
    
    auto val = std::make_shared<Value>(elements);
    // Mark as LIST type (would need to modify Value constructor)
    return val;
}

std::shared_ptr<Value> Parser::parseMap()
{
    if (!match(TokenType::LBRACE))
    {
        error("Expected '{' at start of map");
        return nullptr;
    }
    
    Value::MapType map;
    
    while (!check(TokenType::RBRACE) && !isAtEnd())
    {
        Token key = advance();
        if (key.type != TokenType::IDENTIFIER && key.type != TokenType::STRING)
        {
            error("Expected key in map");
            return nullptr;
        }
        
        if (!match(TokenType::COLON))
        {
            error("Expected ':' after map key");
            return nullptr;
        }
        
        auto value = parseValue();
        if (!value)
        {
            return nullptr;
        }
        
        std::string key_str = (key.type == TokenType::IDENTIFIER) 
            ? key.getValue<std::string>()
            : key.getValue<std::string>();
        
        map[key_str] = value;
        
        if (!match(TokenType::COMMA))
        {
            break;
        }
    }
    
    if (!match(TokenType::RBRACE))
    {
        error("Expected '}' at end of map");
        return nullptr;
    }
    
    return std::make_shared<Value>(map);
}

std::shared_ptr<Value> Parser::parseSet()
{
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' at start of set");
        return nullptr;
    }
    
    Value::ArrayType elements;
    
    while (!check(TokenType::RPAREN) && !isAtEnd())
    {
        auto element = parseValue();
        if (!element)
        {
            return nullptr;
        }
        
        elements.push_back(element);
        
        if (!match(TokenType::COMMA))
        {
            break;
        }
    }
    
    if (!match(TokenType::RPAREN))
    {
        error("Expected ')' at end of set");
        return nullptr;
    }
    
    // For now, represent sets as arrays
    return std::make_shared<Value>(elements);
}

std::shared_ptr<Value> Parser::parseColor()
{
    Token token = peek();
    
    // Hex color #RRGGBB
    if (token.type == TokenType::COLOR && token.hasValue())
    {
        // Check if the value is a hex color string
        std::string hex = token.getValue<std::string>();
        if (hex[0] == '#')
        {
            advance();
            
            // Parse #RRGGBB or #RRGGBBAA
            if (hex.length() >= 7)
            {
                uint8_t r = std::stoi(hex.substr(1, 2), nullptr, 16);
                uint8_t g = std::stoi(hex.substr(3, 2), nullptr, 16);
                uint8_t b = std::stoi(hex.substr(5, 2), nullptr, 16);
                std::optional<uint8_t> a = std::nullopt;
                
                if (hex.length() >= 9)
                {
                    a = std::stoi(hex.substr(7, 2), nullptr, 16);
                }
                
                return std::make_shared<Value>(Color(r, g, b, a));
            }
        }
    }
    
    // Color(r, g, b) or color(r, g, b) constructor
    if (token.type == TokenType::COLOR)
    {
        advance();
        
        if (!match(TokenType::LPAREN))
        {
            error("Expected '(' after Color");
            return nullptr;
        }
        
        // Parse r, g, b components
        Token r_token = advance();
        if (r_token.type != TokenType::INTEGER)
        {
            error("Expected integer for red component");
            return nullptr;
        }
        
        if (!match(TokenType::COMMA))
        {
            error("Expected ',' in color");
            return nullptr;
        }
        
        Token g_token = advance();
        if (g_token.type != TokenType::INTEGER)
        {
            error("Expected integer for green component");
            return nullptr;
        }
        
        if (!match(TokenType::COMMA))
        {
            error("Expected ',' in color");
            return nullptr;
        }
        
        Token b_token = advance();
        if (b_token.type != TokenType::INTEGER)
        {
            error("Expected integer for blue component");
            return nullptr;
        }
        
        if (!match(TokenType::RPAREN))
        {
            error("Expected ')' at end of color");
            return nullptr;
        }
        
        uint8_t r = static_cast<uint8_t>(r_token.getValue<int64_t>());
        uint8_t g = static_cast<uint8_t>(g_token.getValue<int64_t>());
        uint8_t b = static_cast<uint8_t>(b_token.getValue<int64_t>());
        
        return std::make_shared<Value>(Color(r, g, b));
    }
    
    error("Invalid color syntax");
    return nullptr;
}

std::shared_ptr<Value> Parser::parseCoord()
{
    advance(); // Coord keyword
    
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' after Coord");
        return nullptr;
    }
    
    // Parse x, y, optional z
    auto x_val = parseValue();
    if (!x_val || (!x_val->isInteger() && !x_val->isFloat()))
    {
        error("Expected numeric value for x coordinate");
        return nullptr;
    }
    
    if (!match(TokenType::COMMA))
    {
        error("Expected ',' in coord");
        return nullptr;
    }
    
    auto y_val = parseValue();
    if (!y_val || (!y_val->isInteger() && !y_val->isFloat()))
    {
        error("Expected numeric value for y coordinate");
        return nullptr;
    }
    
    double x = x_val->isFloat() ? x_val->asFloat() : x_val->asInteger();
    double y = y_val->isFloat() ? y_val->asFloat() : y_val->asInteger();
    std::optional<double> z = std::nullopt;
    
    // Check for z coordinate
    if (match(TokenType::COMMA))
    {
        auto z_val = parseValue();
        if (!z_val || (!z_val->isInteger() && !z_val->isFloat()))
        {
            error("Expected numeric value for z coordinate");
            return nullptr;
        }
        z = z_val->isFloat() ? z_val->asFloat() : z_val->asInteger();
    }
    
    if (!match(TokenType::RPAREN))
    {
        error("Expected ')' at end of coord");
        return nullptr;
    }
    
    return std::make_shared<Value>(Coord(x, y, z));
}

std::shared_ptr<Value> Parser::parsePath()
{
    advance(); // Path keyword
    
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' after Path");
        return nullptr;
    }
    
    Token path_token = advance();
    if (path_token.type != TokenType::STRING)
    {
        error("Expected string path");
        return nullptr;
    }
    
    if (!match(TokenType::RPAREN))
    {
        error("Expected ')' at end of path");
        return nullptr;
    }
    
    auto val = std::make_shared<Value>(path_token.getValue<std::string>());
    // Mark as PATH type (would need custom handling)
    return val;
}

std::shared_ptr<Value> Parser::parseDynamic()
{
    advance(); // Dyna keyword
    
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' after Dyna");
        return nullptr;
    }
    
    auto inner = parseValue();
    if (!inner)
    {
        return nullptr;
    }
    
    if (!match(TokenType::RPAREN))
    {
        error("Expected ')' at end of Dyna");
        return nullptr;
    }
    
    return Value::makeDynamic(inner);
}

std::shared_ptr<Value> Parser::parseReference()
{
    if (match(TokenType::AT))
    {
        // Simple reference: @name
        Token name = advance();
        if (name.type != TokenType::IDENTIFIER)
        {
            error("Expected identifier after @");
            return nullptr;
        }
        
        return Value::makeReference(name.getValue<std::string>());
    }
    else if (match(TokenType::AT_LBRACE))
    {
        // Cross-section reference: @{section.key} or @{section.key.subkey}
        std::string ref;
        
        Token ident = advance();
        if (ident.type != TokenType::IDENTIFIER)
        {
            error("Expected identifier in cross-section reference");
            return nullptr;
        }
        
        ref = ident.getValue<std::string>();
        
        // Support dot notation for nested access
        while (match(TokenType::DOT))
        {
            Token next = advance();
            if (next.type != TokenType::IDENTIFIER)
            {
                error("Expected identifier after '.' in cross-section reference");
                return nullptr;
            }
            ref += "." + next.getValue<std::string>();
        }
        
        if (!match(TokenType::RBRACE))
        {
            error("Expected '}' at end of reference");
            return nullptr;
        }
        
        return Value::makeReference(ref);
    }
    
    error("Invalid reference syntax");
    return nullptr;
}

std::shared_ptr<Value> Parser::parseEnvVar()
{
    if (!match(TokenType::DOLLAR_LBRACE))
    {
        error("Expected '${' at start of environment variable");
        return nullptr;
    }
    
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER)
    {
        error("Expected identifier in environment variable");
        return nullptr;
    }
    
    if (!match(TokenType::RBRACE))
    {
        error("Expected '}' at end of environment variable");
        return nullptr;
    }
    
    return Value::makeEnvVar(name.getValue<std::string>());
}

void Parser::resolveInheritance()
{
    // Resolve inheritance for each section
    for (auto& [name, section] : sections)
    {
        if (section.inherited_sections.empty())
        {
            continue;
        }
        
        // Merge inherited entries
        std::map<std::string, std::shared_ptr<Value>> merged_entries;
        
        for (const auto& inherited_name : section.inherited_sections)
        {
            auto it = sections.find(inherited_name);
            if (it != sections.end())
            {
                // Copy entries from inherited section
                for (const auto& [key, value] : it->second.entries)
                {
                    merged_entries[key] = value;
                }
            }
        }
        
        // Overlay current section's entries
        for (const auto& [key, value] : section.entries)
        {
            merged_entries[key] = value;
        }
        
        section.entries = merged_entries;
    }
}

bool Parser::validateAgainstSchema()
{
    if (schema.empty())
    {
        return true; // No schema defined, validation passes
    }
    
    // Validate each section against its schema rules
    for (const auto& [schema_section, rules] : schema)
    {
        auto section_it = sections.find(schema_section);
        
        // Check if required section exists
        if (section_it == sections.end())
        {
            // Section doesn't exist, check if any rules are required
            bool has_required = false;
            for (const auto& [key, rule] : rules)
            {
                if (rule.required)
                {
                    has_required = true;
                    break;
                }
            }
            
            if (has_required)
            {
                error("Schema validation failed: Required section [" + schema_section + "] not found");
                return false;
            }
            continue;
        }
        
        auto& section = section_it->second;
        
        // Validate each key in the schema
        for (const auto& [key, rule] : rules)
        {
            auto entry_it = section.entries.find(key);
            
            // Check if required key exists
            if (entry_it == section.entries.end())
            {
                if (rule.required)
                {
                    error("Schema validation failed: Required key '" + key + "' not found in section [" + schema_section + "]");
                    return false;
                }
                
                // Apply default value if specified
                if (rule.null_behavior == SchemaRule::NullBehavior::DEFAULT && rule.default_value)
                {
                    section.entries[key] = rule.default_value;
                }
                else if (rule.null_behavior == SchemaRule::NullBehavior::ERROR)
                {
                    error("Schema validation failed: Key '" + key + "' is null in section [" + schema_section + "]");
                    return false;
                }
                
                continue;
            }
            
            auto& value = entry_it->second;
            
            // Validate value type if specified
            if (rule.value_type.has_value())
            {
                ValueType expected_type = rule.value_type.value();
                ValueType actual_type = value->getType();
                
                if (expected_type != actual_type)
                {
                    error("Schema validation failed: Key '" + key + "' in section [" + schema_section + 
                          "] has wrong type (expected type, got type)");
                    return false;
                }
            }
        }
    }
    
    return true;
}

// Token management

Token Parser::peek() const
{
    if (current >= tokens.size())
    {
        return Token(TokenType::END_OF_FILE);
    }
    return tokens[current];
}

Token Parser::advance()
{
    if (!isAtEnd())
    {
        current++;
    }
    return tokens[current - 1];
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd())
    {
        return false;
    }
    return peek().type == type;
}

bool Parser::isAtEnd() const
{
    if (current >= tokens.size())
    {
        return true;
    }
    return tokens[current].type == TokenType::END_OF_FILE;
}

void Parser::error(const std::string& message)
{
    Token token = peek();
    last_error = "Parse error at line " + std::to_string(token.line) + 
                 ", column " + std::to_string(token.column) + ": " + message;
}

} // namespace yini
