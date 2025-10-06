#include "Parser.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>

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
    
    // Resolve references
    if (!resolveReferences())
    {
        return false;
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
    section.token = name_token;
    
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

    while (match(TokenType::NEWLINE)) {}

    // A schema block continues until the next directive `[#...` or EOF.
    while (!isAtEnd())
    {
        // Stop if we encounter the next directive.
        if (check(TokenType::LBRACKET) && current + 1 < tokens.size() && tokens[current + 1].type == TokenType::HASH)
        {
            break;
        }

        // If it's not a section definition, the schema block is over.
        if (!check(TokenType::LBRACKET))
        {
            break;
        }

        // Consume the section header (e.g., `[Visual]`)
        match(TokenType::LBRACKET);
        Token section_name_token = advance();
        if (section_name_token.type != TokenType::IDENTIFIER)
        {
            error("Expected section name in schema definition");
            return false;
        }
        if (!match(TokenType::RBRACKET))
        {
            error("Expected ']' after section name in schema definition");
            return false;
        }
        std::string sec_name = section_name_token.getValue<std::string>();
        std::map<std::string, SchemaRule> rules;

        while (match(TokenType::NEWLINE)) {}

        // Parse rules for this section. A rule must start with an identifier.
        while (check(TokenType::IDENTIFIER))
        {
            Token key = advance();
            if (!match(TokenType::EQUALS))
            {
                error("Expected '=' after key in schema rule");
                return false;
            }

            SchemaRule rule;
            rule.required = false;
            rule.null_behavior = SchemaRule::NullBehavior::IGNORE;

            // Loop over rule components until a newline is found.
            bool in_rule = true;
            while(in_rule && !isAtEnd() && !check(TokenType::NEWLINE) && !check(TokenType::LBRACKET))
            {
                // Consume optional comma separators
                if (match(TokenType::COMMA)) continue;

                Token component = peek();
                switch (component.type)
                {
                    case TokenType::EXCLAMATION:
                        rule.required = true;
                        advance();
                        break;
                    case TokenType::QUESTION:
                        rule.required = false;
                        advance();
                        break;
                    case TokenType::IDENTIFIER:
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
                        else if (type_str == "e") rule.null_behavior = SchemaRule::NullBehavior::ERROR;
                        else {
                            // This is not a valid type identifier, so the rule must be over.
                            in_rule = false;
                        }
                        if (in_rule) advance();
                        break;
                    }
                    case TokenType::TILDE:
                        rule.null_behavior = SchemaRule::NullBehavior::IGNORE;
                        advance();
                        break;
                    case TokenType::EQUALS:
                        advance(); // Consume '='
                        rule.null_behavior = SchemaRule::NullBehavior::DEFAULT;
                        rule.default_value = parseValue();
                        if (!rule.default_value) {
                             error("Failed to parse default value in schema rule");
                             return false;
                        }
                        break;
                    default:
                        // Any other token ends the parsing of this rule.
                        in_rule = false;
                        break;
                }
            }
            rules[key.getValue<std::string>()] = rule;

            // Consume the newline at the end of the rule.
            if (!isAtEnd() && !check(TokenType::LBRACKET)) {
                if (!match(TokenType::NEWLINE)) {
                    error("Expected newline after schema rule");
                    return false;
                }
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
            left = std::make_shared<Value>(result, left->getToken());
        }
        else if ((left->isFloat() || left->isInteger()) && 
                 (right->isFloat() || right->isInteger()))
        {
            double l_val = left->isFloat() ? left->asFloat() : left->asInteger();
            double r_val = right->isFloat() ? right->asFloat() : right->asInteger();
            double result = (op == TokenType::PLUS) ? (l_val + r_val) : (l_val - r_val);
            left = std::make_shared<Value>(result, left->getToken());
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
            left = std::make_shared<Value>(result, left->getToken());
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
            left = std::make_shared<Value>(result, left->getToken());
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
            return std::make_shared<Value>(-value->asInteger(), value->getToken());
        }
        else if (value->isFloat())
        {
            return std::make_shared<Value>(-value->asFloat(), value->getToken());
        }
        else
        {
            error("Cannot negate non-numeric value");
            return nullptr;
        }
    }
    
    // Parenthesized expressions are now handled in parseSet to resolve ambiguity
    // with tuples/sets.
    return parsePrimary();
}

std::shared_ptr<Value> Parser::parsePrimary()
{
    Token token = peek();
    
    // Literals
    if (token.type == TokenType::INTEGER)
    {
        advance();
        return std::make_shared<Value>(token.getValue<int64_t>(), token);
    }
    
    if (token.type == TokenType::FLOAT)
    {
        advance();
        return std::make_shared<Value>(token.getValue<double>(), token);
    }
    
    if (token.type == TokenType::BOOLEAN)
    {
        advance();
        return std::make_shared<Value>(token.getValue<bool>(), token);
    }
    
    if (token.type == TokenType::STRING)
    {
        advance();
        return std::make_shared<Value>(token.getValue<std::string>(), token);
    }
    
    // Arrays
    if (token.type == TokenType::LBRACKET)
    {
        return parseArray(token);
    }
    
    // Maps/tuples
    if (token.type == TokenType::LBRACE)
    {
        return parseMap(token);
    }
    
    // Sets, Tuples, and Grouped Expressions
    if (token.type == TokenType::LPAREN)
    {
        return parseSet(token);
    }
    
    // Hex color or Color() constructor
    if (token.type == TokenType::COLOR)
    {
        return parseColor(token);
    }
    
    if (token.type == TokenType::COORD)
    {
        return parseCoord(token);
    }
    
    if (token.type == TokenType::PATH)
    {
        return parsePath(token);
    }
    
    if (token.type == TokenType::LIST)
    {
        return parseList(token);
    }
    
    if (token.type == TokenType::ARRAY)
    {
        Token array_token = advance(); // Array keyword
        if (!match(TokenType::LPAREN))
        {
            error("Expected '(' after Array");
            return nullptr;
        }
        // We pass the `Array(` token for position, but use `[` for parsing logic.
        Token bracket_token(TokenType::LBRACKET, array_token.line, array_token.column);
        auto arr = parseArray(bracket_token);
        // TODO: We could potentially mark this as a different kind of array if needed.
        return arr;
    }
    
    if (token.type == TokenType::DYNA)
    {
        return parseDynamic(token);
    }
    
    // References
    if (token.type == TokenType::AT)
    {
        return parseReference(token);
    }
    
    if (token.type == TokenType::AT_LBRACE)
    {
        return parseReference(token);
    }
    
    // Environment variables
    if (token.type == TokenType::DOLLAR_LBRACE)
    {
        return parseEnvVar(token);
    }
    
    error("Unexpected token in value: " + token.toString());
    return nullptr;
}

std::shared_ptr<Value> Parser::parseArray(Token token)
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
    
    return std::make_shared<Value>(elements, token);
}

std::shared_ptr<Value> Parser::parseList(Token token)
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
    
    // A List is explicitly a List.
    return Value::makeList(elements, token);
}

std::shared_ptr<Value> Parser::parseMap(Token token)
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
    
    return std::make_shared<Value>(map, token);
}

// This function now handles grouped expressions, sets, and tuples to resolve ambiguity.
std::shared_ptr<Value> Parser::parseSet(Token token)
{
    if (!match(TokenType::LPAREN))
    {
        error("Expected '(' at start of grouping or tuple/set");
        return nullptr;
    }

    // Handle empty tuple `()`
    if (check(TokenType::RPAREN)) {
        advance(); // consume ')'
        return Value::makeTuple({}, token);
    }
    
    Value::ArrayType elements;
    elements.push_back(parseExpression());

    // If a comma follows the first expression, it's a tuple/set.
    if (match(TokenType::COMMA)) {
        // It's a tuple/set like `(a,)` or `(a, b, ...)`.
        // The comma for `(a,)` is already consumed. If there are more elements, parse them.
        while (!check(TokenType::RPAREN) && !isAtEnd()) {
            elements.push_back(parseExpression());
            if (!match(TokenType::COMMA)) {
                break; // No more commas, so no more elements.
            }
        }

        if (!match(TokenType::RPAREN)) {
            error("Expected ')' after elements in tuple/set");
            return nullptr;
        }

        // Distinguish between a Set (all same type) and a Tuple (mixed types).
        bool all_same_type = true;
        ValueType first_type = elements[0]->getType();
        for (size_t i = 1; i < elements.size(); ++i)
        {
            if (elements[i]->getType() != first_type)
            {
                all_same_type = false;
                break;
            }
        }

        if (all_same_type) {
            return Value::makeSet(elements, token);
        } else {
            return Value::makeTuple(elements, token);
        }
    }
    // If no comma follows, it must be a grouped expression like `(expr)`.
    else if (match(TokenType::RPAREN))
    {
        return elements[0];
    }
    else
    {
        error("Expected ')' or ',' after expression in parentheses");
        return nullptr;
    }
}

std::shared_ptr<Value> Parser::parseColor(Token token)
{
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
                
                return std::make_shared<Value>(Color(r, g, b, a), token);
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
        
        return std::make_shared<Value>(Color(r, g, b), token);
    }
    
    error("Invalid color syntax");
    return nullptr;
}

std::shared_ptr<Value> Parser::parseCoord(Token token)
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
    
    return std::make_shared<Value>(Coord(x, y, z), token);
}

std::shared_ptr<Value> Parser::parsePath(Token token)
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
    
    // A Path is explicitly a Path.
    return Value::makePath(path_token.getValue<std::string>(), token);
}

std::shared_ptr<Value> Parser::parseDynamic(Token token)
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
    
    return Value::makeDynamic(inner, token);
}

std::shared_ptr<Value> Parser::parseReference(Token token)
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
        
        return Value::makeReference(name.getValue<std::string>(), token);
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
        
        return Value::makeReference(ref, token);
    }
    
    error("Invalid reference syntax");
    return nullptr;
}

std::shared_ptr<Value> Parser::parseEnvVar(Token token)
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
    
    return Value::makeEnvVar(name.getValue<std::string>(), token);
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
                    // Error: required key is missing. We don't have a token for something that doesn't exist,
                    // so we report the error at the section level.
                    error("Schema validation failed: Required key '" + key + "' not found in section [" + schema_section + "]", section.token);
                    return false;
                }
                
                // Apply default value if specified
                if (rule.null_behavior == SchemaRule::NullBehavior::DEFAULT && rule.default_value)
                {
                    section.entries[key] = rule.default_value;
                }
                else if (rule.null_behavior == SchemaRule::NullBehavior::ERROR)
                {
                    error("Schema validation failed: Key '" + key + "' is null in section [" + schema_section + "]", section.token);
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
                    // Now we can report the error with the exact location of the problematic value.
                    error("Schema validation failed: Key '" + key + "' in section [" + schema_section + 
                          "] has wrong type", value->getToken());
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool Parser::resolveReferences()
{
    // Resolve all references in all sections
    for (auto& [section_name, section] : sections)
    {
        for (auto& [key, value] : section.entries)
        {
            std::set<std::string> visiting;
            std::string ref_path = section_name + "." + key;
            visiting.insert(ref_path);
            
            auto resolved = resolveValue(value, visiting);
            if (!resolved)
            {
                // If a more specific error was not already set by resolveValue, set a generic one.
                if (!hasError())
                {
                    error("Failed to resolve reference in [" + section_name + "]." + key);
                }
                return false;
            }
            
            section.entries[key] = resolved;
        }
    }
    
    return true;
}

std::shared_ptr<Value> Parser::resolveValue(std::shared_ptr<Value> value, std::set<std::string>& visiting)
{
    if (!value)
    {
        return value;
    }
    
    // Handle reference types
    if (value->isReference())
    {
        std::string ref_name = value->asString();
        
        // Check for macro reference first (@name)
        if (defines.find(ref_name) != defines.end())
        {
            // Resolve macro
            auto macro_value = defines[ref_name];
            
            // Recursively resolve the macro value
            auto resolved = resolveValue(macro_value, visiting);
            return resolved;
        }
        
        // Check for cross-section reference (@{Section.key})
        // Format: "Section.key" or "Section.key.subkey"
        size_t dot_pos = ref_name.find('.');
        if (dot_pos != std::string::npos)
        {
            std::string section_name = ref_name.substr(0, dot_pos);
            std::string key_name = ref_name.substr(dot_pos + 1);
            
            // Check for circular reference
            std::string ref_path = ref_name;
            if (visiting.find(ref_path) != visiting.end())
            {
                error("Circular reference detected: " + ref_path);
                return nullptr;
            }
            
            // Find the section
            auto section_it = sections.find(section_name);
            if (section_it == sections.end())
            {
                error("Reference to unknown section: " + section_name);
                return nullptr;
            }
            
            // Find the key
            auto& section = section_it->second;
            auto entry_it = section.entries.find(key_name);
            if (entry_it == section.entries.end())
            {
                error("Reference to unknown key: " + key_name + " in section [" + section_name + "]");
                return nullptr;
            }
            
            // Add to visiting set
            visiting.insert(ref_path);
            
            // Recursively resolve
            auto resolved = resolveValue(entry_it->second, visiting);
            
            // Remove from visiting set
            visiting.erase(ref_path);
            
            return resolved;
        }
        
        // Unknown reference
        error("Unresolved reference: " + ref_name);
        return nullptr;
    }
    
    // Handle environment variables
    if (value->isEnvVar())
    {
        std::string var_name = value->asString();
        const char* env_value = std::getenv(var_name.c_str());
        
        if (env_value)
        {
            return std::make_shared<Value>(std::string(env_value), value->getToken());
        }
        else
        {
            // Environment variable not set, return empty string
            return std::make_shared<Value>(std::string(""), value->getToken());
        }
    }
    
    // Handle arrays (recursively resolve elements)
    if (value->isArray())
    {
        auto arr = value->asArray();
        Value::ArrayType resolved_arr;
        
        for (auto& elem : arr)
        {
            auto resolved_elem = resolveValue(elem, visiting);
            if (!resolved_elem)
            {
                return nullptr;
            }
            resolved_arr.push_back(resolved_elem);
        }
        
        return std::make_shared<Value>(resolved_arr, value->getToken());
    }
    
    // Handle maps (recursively resolve values)
    if (value->isMap())
    {
        auto map = value->asMap();
        Value::MapType resolved_map;
        
        for (auto& [k, v] : map)
        {
            auto resolved_v = resolveValue(v, visiting);
            if (!resolved_v)
            {
                return nullptr;
            }
            resolved_map[k] = resolved_v;
        }
        
        return std::make_shared<Value>(resolved_map, value->getToken());
    }
    
    // For all other types, return as-is
    return value;
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
    // Only record the first error encountered.
    if (!hasError())
    {
        Token token = peek();
        last_error = "Parse error at line " + std::to_string(token.line) +
                     ", column " + std::to_string(token.column) + ": " + message;
    }
}

void Parser::error(const std::string& message, const Token& token)
{
    // Only record the first error encountered.
    if (!hasError())
    {
        last_error = "Parse error at line " + std::to_string(token.line) +
                     ", column " + std::to_string(token.column) + ": " + message;
    }
}

} // namespace yini
