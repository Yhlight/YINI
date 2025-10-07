#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Parser.h"

namespace YINI
{
    Parser::Parser(Lexer& lexer, const std::string& filepath)
        : m_lexer(lexer), m_filepath(filepath)
    {
        advance();
    }

    std::unique_ptr<AstNode> Parser::parse()
    {
        return std::make_unique<AstNode>(parseAst());
    }

    void Parser::advance()
    {
        m_currentToken = m_lexer.nextToken();
    }

    bool Parser::match(TokenType type)
    {
        if (m_currentToken.type == type)
        {
            advance();
            return true;
        }
        return false;
    }

    void Parser::consume(TokenType type, const std::string& message)
    {
        if (m_currentToken.type == type)
        {
            advance();
        }
        else
        {
            throw std::runtime_error(message);
        }
    }

    AstNode Parser::parseAst()
    {
        AstNode ast;
        while (m_currentToken.type != TokenType::EndOfFile)
        {
            if (m_currentToken.type == TokenType::LeftBracket)
            {
                Token next = m_lexer.peek();
                if (next.type == TokenType::Hash)
                {
                    parseSpecialSection(ast);
                }
                else
                {
                    ast.sections.push_back(parseSection(ast));
                }
            } else {
                advance(); // Skip unexpected tokens at the top level
            }
        }
        return ast;
    }

    void Parser::parseSpecialSection(AstNode& ast)
    {
        consume(TokenType::LeftBracket, "Expected '[' at the start of a special section.");
        consume(TokenType::Hash, "Expected '#' at the start of a special section.");

        std::string section_type = m_currentToken.text;
        consume(TokenType::Identifier, "Expected special section type (e.g., 'define', 'include').");
        consume(TokenType::RightBracket, "Expected ']' at the end of a special section.");

        if (section_type == "define")
        {
            while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::EndOfFile)
            {
                KeyValueNode kv = parseKeyValue(ast);
                ast.macros[kv.key] = std::move(kv.value);
            }
        }
        else if (section_type == "include")
        {
            while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::EndOfFile)
            {
                if (match(TokenType::PlusEquals))
                {
                    if (m_currentToken.type != TokenType::String)
                    {
                        throw std::runtime_error("Include path must be a string.");
                    }
                    std::string include_path_str = m_currentToken.text;
                    advance();

                    std::filesystem::path base_path(m_filepath);
                    std::filesystem::path full_path = base_path.parent_path() / include_path_str;

                    std::ifstream file(full_path);
                    if (!file.is_open()) {
                        throw std::runtime_error("Could not open included file: " + full_path.string());
                    }

                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string content = buffer.str();

                    Lexer included_lexer(content);
                    Parser included_parser(included_lexer, full_path.string());
                    std::unique_ptr<AstNode> included_ast = included_parser.parse();

                    // Merge included AST
                    for (auto& [key, val] : included_ast->macros) {
                        ast.macros[key] = std::move(val);
                    }
                    for (auto& included_section : included_ast->sections) {
                        bool section_found = false;
                        for (auto& existing_section : ast.sections) {
                            if (existing_section.name == included_section.name) {
                                for (auto& kv : included_section.key_values) {
                                    bool key_found = false;
                                    for (auto& existing_kv : existing_section.key_values) {
                                        if (existing_kv.key == kv.key) {
                                            existing_kv.value = std::move(kv.value);
                                            key_found = true;
                                            break;
                                        }
                                    }
                                    if (!key_found) {
                                        existing_section.key_values.push_back(std::move(kv));
                                    }
                                }
                                section_found = true;
                                break;
                            }
                        }
                        if (!section_found) {
                            ast.sections.push_back(std::move(included_section));
                        }
                    }
                } else {
                    throw std::runtime_error("Expected '+=' for include paths.");
                }
            }
        }
        else
        {
            throw std::runtime_error("Unknown special section type: " + section_type);
        }
    }

    SectionNode Parser::parseSection(AstNode& ast)
    {
        SectionNode section;
        m_plusEqualsCounter = 0;
        consume(TokenType::LeftBracket, "Expected '[' at the start of a section.");
        section.name = m_currentToken.text;
        consume(TokenType::Identifier, "Expected section name.");

        if (match(TokenType::Colon))
        {
            do
            {
                section.parents.push_back(m_currentToken.text);
                consume(TokenType::Identifier, "Expected parent name.");
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightBracket, "Expected ']' at the end of a section.");

        while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::EndOfFile)
        {
            section.key_values.push_back(parseKeyValue(ast));
        }

        return section;
    }

    KeyValueNode Parser::parseKeyValue(AstNode& ast)
    {
        KeyValueNode kv;

        if (match(TokenType::PlusEquals))
        {
            kv.key = std::to_string(m_plusEqualsCounter++);
            kv.value = parseValue(ast);
        }
        else
        {
            kv.key = m_currentToken.text;
            consume(TokenType::Identifier, "Expected key.");
            consume(TokenType::Equals, "Expected '=' after key.");
            kv.value = parseValue(ast);
        }

        return kv;
    }

#include <optional>

    // Helper function to extract a numeric value from a YiniValue
    double get_numeric_value(const YiniValue& val) {
        if (const auto* d = std::get_if<double>(&val.value)) {
            return *d;
        }
        if (const auto* i = std::get_if<int64_t>(&val.value)) {
            return static_cast<double>(*i);
        }
        throw std::runtime_error("Expected a numeric argument.");
    }

    // Helper function for arithmetic operations
    template<typename T, typename U>
    std::unique_ptr<YiniValue> apply_op(TokenType op, T left, U right) {
        auto result = std::make_unique<YiniValue>();
        if constexpr (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
            double l = static_cast<double>(left);
            double r = static_cast<double>(right);
            switch (op) {
                case TokenType::Plus: result->value = l + r; break;
                case TokenType::Minus: result->value = l - r; break;
                case TokenType::Star: result->value = l * r; break;
                case TokenType::Slash: result->value = l / r; break;
                default: throw std::runtime_error("Invalid floating point operator.");
            }
        } else {
            int64_t l = static_cast<int64_t>(left);
            int64_t r = static_cast<int64_t>(right);
            switch (op) {
                case TokenType::Plus: result->value = l + r; break;
                case TokenType::Minus: result->value = l - r; break;
                case TokenType::Star: result->value = l * r; break;
                case TokenType::Slash: result->value = l / r; break;
                case TokenType::Percent: result->value = l % r; break;
                default: throw std::runtime_error("Invalid integer operator.");
            }
        }
        return result;
    }

    std::unique_ptr<YiniValue> Parser::parseValue(AstNode& ast)
    {
        return parseExpression(ast);
    }

    std::unique_ptr<YiniValue> Parser::parseExpression(AstNode& ast)
    {
        auto left = parseTerm(ast);

        while (m_currentToken.type == TokenType::Plus || m_currentToken.type == TokenType::Minus)
        {
            Token op = m_currentToken;
            advance(); // Consume the operator
            auto right = parseTerm(ast);
            left = std::visit([&](auto& lval, auto& rval) -> std::unique_ptr<YiniValue> {
                using L_t = std::decay_t<decltype(lval)>;
                using R_t = std::decay_t<decltype(rval)>;
                if constexpr ((std::is_same_v<L_t, int64_t> || std::is_same_v<L_t, double>) &&
                              (std::is_same_v<R_t, int64_t> || std::is_same_v<R_t, double>)) {
                    return apply_op(op.type, lval, rval);
                } else {
                    throw std::runtime_error("Arithmetic operation on non-numeric types.");
                }
            }, left->value, right->value);
        }

        return left;
    }

    std::unique_ptr<YiniValue> Parser::parseTerm(AstNode& ast)
    {
        auto left = parseFactor(ast);

        while (m_currentToken.type == TokenType::Star || m_currentToken.type == TokenType::Slash || m_currentToken.type == TokenType::Percent)
        {
            Token op = m_currentToken;
            advance(); // Consume the operator
            auto right = parseFactor(ast);
            left = std::visit([&](auto& lval, auto& rval) -> std::unique_ptr<YiniValue> {
                using L_t = std::decay_t<decltype(lval)>;
                using R_t = std::decay_t<decltype(rval)>;
                if constexpr ((std::is_same_v<L_t, int64_t> || std::is_same_v<L_t, double>) &&
                              (std::is_same_v<R_t, int64_t> || std::is_same_v<R_t, double>)) {
                    return apply_op(op.type, lval, rval);
                } else {
                    throw std::runtime_error("Arithmetic operation on non-numeric types.");
                }
            }, left->value, right->value);
        }

        return left;
    }

    std::unique_ptr<YiniValue> Parser::parseFactor(AstNode& ast)
    {
        if (m_currentToken.type == TokenType::Minus)
        {
            advance(); // consume '-'
            auto val = parseFactor(ast);
            return std::visit([](auto& v) -> std::unique_ptr<YiniValue> {
                auto result = std::make_unique<YiniValue>();
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, int64_t>) {
                    result->value = -v;
                } else if constexpr (std::is_same_v<T, double>) {
                    result->value = -v;
                } else {
                    throw std::runtime_error("Unary minus on non-numeric type.");
                }
                return result;
            }, val->value);
        }
        return parsePrimary(ast);
    }

    std::unique_ptr<YiniValue> Parser::parsePrimary(AstNode& ast)
    {
        switch (m_currentToken.type)
        {
            case TokenType::Integer: {
                auto val = std::make_unique<YiniValue>();
                val->value = std::stoll(m_currentToken.text);
                advance();
                return val;
            }
            case TokenType::Float: {
                auto val = std::make_unique<YiniValue>();
                val->value = std::stod(m_currentToken.text);
                advance();
                return val;
            }
            case TokenType::LeftParen: {
                advance(); // consume '('

                // Handle empty set '()'
                if (m_currentToken.type == TokenType::RightParen) {
                    advance(); // consume ')'
                    auto array = std::make_unique<YiniValue::Array>();
                    auto result = std::make_unique<YiniValue>();
                    result->value = std::move(array);
                    return result;
                }

                auto first_val = parseExpression(ast);

                if (match(TokenType::Comma)) {
                    // This is a set/tuple, represented as an array
                    auto array = std::make_unique<YiniValue::Array>();
                    array->push_back(std::move(*first_val));

                    // Handle trailing comma for single-element set, e.g. (elem,)
                    if (m_currentToken.type == TokenType::RightParen) {
                        advance(); // consume ')'
                        auto result = std::make_unique<YiniValue>();
                        result->value = std::move(array);
                        return result;
                    }

                    do {
                        array->push_back(std::move(*parseExpression(ast)));
                    } while (match(TokenType::Comma));

                    consume(TokenType::RightParen, "Expected ')' to end a set.");
                    auto result = std::make_unique<YiniValue>();
                    result->value = std::move(array);
                    return result;

                } else {
                    // This was a grouped expression
                    consume(TokenType::RightParen, "Expected ')' after expression.");
                    return first_val;
                }
            }
            case TokenType::At: {
                advance(); // consume '@'
                if (m_currentToken.type == TokenType::LeftBrace) {
                    // Cross-section reference: @{section.key}
                    advance(); // consume '{'

                    std::string sectionName = m_currentToken.text;
                    consume(TokenType::Identifier, "Expected section name in cross-section reference.");

                    consume(TokenType::Dot, "Expected '.' between section and key.");

                    std::string keyName = m_currentToken.text;
                    consume(TokenType::Identifier, "Expected key name in cross-section reference.");

                    consume(TokenType::RightBrace, "Expected '}' to end cross-section reference.");

                    auto val = std::make_unique<YiniValue>();
                    val->value = CrossSectionRef{sectionName, keyName};
                    return val;
                } else {
                    // Macro reference: @macro
                    std::string macro_name = m_currentToken.text;
                    consume(TokenType::Identifier, "Expected macro name after '@'.");
                    if (ast.macros.count(macro_name)) {
                        return ast.macros.at(macro_name)->clone();
                    } else {
                        throw std::runtime_error("Undefined macro: " + macro_name);
                    }
                }
            }
            case TokenType::String: {
                auto val = std::make_unique<YiniValue>();
                val->value = m_currentToken.text;
                advance();
                return val;
            }
            case TokenType::True: {
                auto val = std::make_unique<YiniValue>();
                val->value = true;
                advance();
                return val;
            }
            case TokenType::False: {
                auto val = std::make_unique<YiniValue>();
                val->value = false;
                advance();
                return val;
            }
            case TokenType::Identifier: {
                std::string name = m_currentToken.text;
                advance();
                if (m_currentToken.type == TokenType::LeftParen)
                {
                    return parseFunctionCall(ast, name);
                }
                auto val = std::make_unique<YiniValue>();
                val->value = name;
                return val;
            }
            case TokenType::LeftBracket: {
                auto val = std::make_unique<YiniValue>();
                val->value = parseArray(ast);
                return val;
            }
            case TokenType::LeftBrace: {
                auto val = std::make_unique<YiniValue>();
                val->value = parseMap(ast);
                return val;
            }
            case TokenType::Hash: {
                advance(); // consume '#'
                if (m_currentToken.type != TokenType::Identifier || m_currentToken.text.length() != 6) {
                    throw std::runtime_error("Invalid hex color format.");
                }
                auto val = std::make_unique<YiniValue>();
                uint32_t hex_val = std::stoul(m_currentToken.text, nullptr, 16);
                val->value = Color{
                    static_cast<uint8_t>((hex_val >> 16) & 0xFF),
                    static_cast<uint8_t>((hex_val >> 8) & 0xFF),
                    static_cast<uint8_t>(hex_val & 0xFF)
                };
                advance(); // consume hex identifier
                return val;
            }
            case TokenType::EnvVar: {
                auto val = std::make_unique<YiniValue>();
                const char* env_val = std::getenv(m_currentToken.text.c_str());
                val->value = env_val ? std::string(env_val) : "";
                advance();
                return val;
            }
            default:
                throw std::runtime_error("Unexpected token in expression: " + m_currentToken.text);
        }
    }

    std::unique_ptr<YiniValue::Array> Parser::parseArray(AstNode& ast)
    {
        auto array = std::make_unique<YiniValue::Array>();
        consume(TokenType::LeftBracket, "Expected '[' to start an array.");

        if (match(TokenType::RightBracket))
        {
            return array;
        }

        do
        {
            array->push_back(std::move(*parseValue(ast)));
        } while (match(TokenType::Comma));

        consume(TokenType::RightBracket, "Expected ']' to end an array.");
        return array;
    }

    std::unique_ptr<YiniValue::Map> Parser::parseMap(AstNode& ast)
    {
        auto map = std::make_unique<YiniValue::Map>();
        consume(TokenType::LeftBrace, "Expected '{' to start a map.");

        if (match(TokenType::RightBrace))
        {
            return map;
        }

        do
        {
            if (m_currentToken.type != TokenType::String)
            {
                throw std::runtime_error("Expected a string key in map.");
            }
            std::string key = m_currentToken.text;
            advance();

            consume(TokenType::Colon, "Expected ':' after map key.");

            map->emplace(std::move(key), std::move(*parseValue(ast)));

        } while (match(TokenType::Comma));

        consume(TokenType::RightBrace, "Expected '}' to end a map.");
        return map;
    }

    std::vector<std::unique_ptr<YiniValue>> Parser::parseArgumentList(AstNode& ast)
    {
        std::vector<std::unique_ptr<YiniValue>> args;
        consume(TokenType::LeftParen, "Expected '(' to start an argument list.");

        if (m_currentToken.type == TokenType::RightParen)
        {
            advance(); // consume ')'
            return args; // Empty argument list
        }

        do
        {
            args.push_back(parseExpression(ast));
        } while (match(TokenType::Comma));

        consume(TokenType::RightParen, "Expected ')' to end an argument list.");
        return args;
    }

    std::unique_ptr<YiniValue> Parser::parseFunctionCall(AstNode& ast, const std::string& functionName)
    {
        auto args = parseArgumentList(ast);
        auto lower_name = functionName;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        if (lower_name == "array" || lower_name == "list")
        {
            auto array = std::make_unique<YiniValue::Array>();
            for (auto& arg : args)
            {
                array->push_back(std::move(*arg));
            }
            auto result = std::make_unique<YiniValue>();
            result->value = std::move(array);
            return result;
        }
        else if (lower_name == "color")
        {
            if (args.size() != 3)
            {
                throw std::runtime_error("Color function requires 3 arguments.");
            }
            auto result = std::make_unique<YiniValue>();
            result->value = Color{
                static_cast<uint8_t>(std::get<int64_t>(args[0]->value)),
                static_cast<uint8_t>(std::get<int64_t>(args[1]->value)),
                static_cast<uint8_t>(std::get<int64_t>(args[2]->value))
            };
            return result;
        }
        else if (lower_name == "coord")
        {
            if (args.size() != 2 && args.size() != 3)
            {
                throw std::runtime_error("Coord function requires 2 or 3 arguments.");
            }
            auto result = std::make_unique<YiniValue>();
            double x = get_numeric_value(*args[0]);
            double y = get_numeric_value(*args[1]);
            std::optional<double> z;
            if (args.size() == 3) {
                z = get_numeric_value(*args[2]);
            }
            result->value = Coord{x, y, z};
            return result;
        }
        else if (lower_name == "path")
        {
            if (args.size() != 1)
            {
                throw std::runtime_error("Path function requires 1 string argument.");
            }
            auto result = std::make_unique<YiniValue>();
            result->value = Path{std::get<std::string>(args[0]->value)};
            return result;
        }
        else if (lower_name == "dyna")
        {
            if (args.size() != 1)
            {
                throw std::runtime_error("Dyna function requires 1 argument.");
            }
            auto result = std::make_unique<YiniValue>();
            result->value = std::move(args[0]);
            return result;
        }

        throw std::runtime_error("Unknown function call: " + functionName);
    }
}