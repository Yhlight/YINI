#include "Parser.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace YINI
{

static std::set<TokenType> key_keywords = {TokenType::TRUE, TokenType::FALSE, TokenType::COLOR, TokenType::COORD,
                                           TokenType::PATH, TokenType::LIST,  TokenType::ARRAY, TokenType::DYNA};

// Helper to trim whitespace from both ends of a string
static void trim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

Parser::Parser(const std::vector<Token> &tokens) : m_tokens(tokens)
{
}

std::vector<std::unique_ptr<AST::Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<AST::Stmt>> statements;
    while (!is_at_end())
    {
        auto stmt = declaration();
        if (stmt)
        { // declaration can return nullptr at EOF
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}

std::unique_ptr<AST::Stmt> Parser::declaration()
{
    if (peek().type != TokenType::LEFT_BRACKET && peek().type != TokenType::END_OF_FILE)
    {
        Token error_token = peek();
        std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                    std::to_string(error_token.column) +
                                    ": Top-level key-value pairs are not allowed. All keys must be inside a section.";
        throw std::runtime_error(error_message);
    }

    if (match({TokenType::LEFT_BRACKET}))
    {
        if (match({TokenType::HASH}))
        {
            Token keyword = consume(TokenType::IDENTIFIER, "Expect 'define', 'include', or 'schema' keyword.");
            if (keyword.lexeme == "define")
            {
                return define_section_declaration();
            }
            else if (keyword.lexeme == "include")
            {
                return include_section_declaration();
            }
            else if (keyword.lexeme == "schema")
            {
                return schema_declaration();
            }
            else
            {
                Token error_token = peek();
                std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                            std::to_string(error_token.column) + ": Unknown directive '#" +
                                            keyword.lexeme + "'.";
                throw std::runtime_error(error_message);
            }
        }
        return section_declaration();
    }

    return nullptr; // End of file
}

std::unique_ptr<AST::Stmt> Parser::section_declaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect section name.");

    auto section = std::make_unique<AST::SectionStmt>();
    section->name = name;

    consume(TokenType::RIGHT_BRACKET, "Expect ']' after section name.");

    if (match({TokenType::COLON}))
    {
        do
        {
            section->parent_sections.push_back(consume(TokenType::IDENTIFIER, "Expect parent section name."));
        } while (match({TokenType::COMMA}));
    }

    while (!check(TokenType::LEFT_BRACKET) && !is_at_end())
    {
        if (peek().type == TokenType::PLUS_EQUAL)
        {
            section->statements.push_back(quick_reg_statement());
        }
        else
        {
            section->statements.push_back(key_value_statement());
        }
    }

    return section;
}

std::unique_ptr<AST::Stmt> Parser::define_section_declaration()
{
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after '#define'.");

    auto define_section = std::make_unique<AST::DefineSectionStmt>();

    while (!check(TokenType::LEFT_BRACKET) && !is_at_end())
    {
        define_section->definitions.push_back(key_value_statement());
    }

    return define_section;
}

std::unique_ptr<AST::Stmt> Parser::include_section_declaration()
{
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after '#include'.");

    auto include_stmt = std::make_unique<AST::IncludeStmt>();

    while (match({TokenType::PLUS_EQUAL}))
    {
        include_stmt->paths.push_back(expression());
    }

    return include_stmt;
}

std::unique_ptr<AST::Stmt> Parser::schema_declaration()
{
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after '#schema'.");
    auto schema = std::make_unique<AST::SchemaStmt>();

    // A [#schema] directive applies to the next section.
    if (peek().type == TokenType::LEFT_BRACKET)
    {
        schema->sections.push_back(schema_section_declaration());
    }

    return schema;
}

std::unique_ptr<AST::SchemaSectionStmt> Parser::schema_section_declaration()
{
    consume(TokenType::LEFT_BRACKET, "Expect '[' to start a schema section.");
    Token name = consume(TokenType::IDENTIFIER, "Expect section name in schema.");
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after schema section name.");

    auto schema_section = std::make_unique<AST::SchemaSectionStmt>();
    schema_section->name = name;

    while (!is_at_end() && peek().type != TokenType::LEFT_BRACKET)
    {
        schema_section->rules.push_back(schema_rule_statement());
    }

    return schema_section;
}

std::unique_ptr<AST::SchemaRuleStmt> Parser::schema_rule_statement()
{
    Token key = consume(TokenType::IDENTIFIER, "Expect key for schema rule.");
    consume(TokenType::EQUAL, "Expect '=' after schema rule key.");

    auto rule_stmt = std::make_unique<AST::SchemaRuleStmt>();
    rule_stmt->key = key;

    // --- New Robust Parsing Logic ---
    // 1. Reconstruct the raw string for the rest of the line.
    std::string rule_string;
    while (!is_at_end() && peek().line == key.line)
    {
        rule_string += advance().lexeme;
    }

    // 2. Split the string by commas and process each part.
    std::stringstream ss(rule_string);
    std::string segment;
    bool type_parsed = false;

    while (std::getline(ss, segment, ','))
    {
        trim(segment);
        if (segment.empty())
            continue;

        if (segment == "!")
        {
            rule_stmt->rule.requirement = AST::SchemaRule::Requirement::REQUIRED;
        }
        else if (segment == "?")
        {
            rule_stmt->rule.requirement = AST::SchemaRule::Requirement::OPTIONAL;
        }
        else if (segment == "~")
        {
            rule_stmt->rule.empty_behavior = AST::SchemaRule::EmptyBehavior::IGNORE;
        }
        else if (segment == "e")
        {
            rule_stmt->rule.empty_behavior = AST::SchemaRule::EmptyBehavior::THROW_ERROR;
        }
        else if (segment.rfind("=", 0) == 0)
        { // starts with '='
            rule_stmt->rule.empty_behavior = AST::SchemaRule::EmptyBehavior::ASSIGN_DEFAULT;
            rule_stmt->rule.default_value = segment.substr(1);
        }
        else if (segment.rfind("min=", 0) == 0)
        {
            rule_stmt->rule.min = std::stod(segment.substr(4));
        }
        else if (segment.rfind("max=", 0) == 0)
        {
            rule_stmt->rule.max = std::stod(segment.substr(4));
        }
        else if (!type_parsed)
        { // Assume it's the type
            if (segment.rfind("array[", 0) == 0 && segment.back() == ']')
            {
                rule_stmt->rule.type = "array";
                rule_stmt->rule.array_subtype = segment.substr(6, segment.length() - 7);
            }
            else
            {
                rule_stmt->rule.type = segment;
            }
            type_parsed = true;
        }
    }

    return rule_stmt;
}

std::unique_ptr<AST::KeyValueStmt> Parser::key_value_statement()
{
    Token key;
    if (check(TokenType::IDENTIFIER) || key_keywords.count(peek().type) || check(TokenType::NUMBER))
    {
        key = advance();
    }
    else
    {
        Token error_token = peek();
        std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                    std::to_string(error_token.column) + ": Expect key.";
        throw std::runtime_error(error_message);
    }

    consume(TokenType::EQUAL, "Expect '=' after key.");
    auto value = expression();

    auto stmt = std::make_unique<AST::KeyValueStmt>();
    stmt->key = key;
    stmt->value = std::move(value);

    return stmt;
}

std::unique_ptr<AST::QuickRegStmt> Parser::quick_reg_statement()
{
    consume(TokenType::PLUS_EQUAL, "Expect '+=' for quick registration.");
    auto value = expression();
    auto stmt = std::make_unique<AST::QuickRegStmt>();
    stmt->value = std::move(value);
    return stmt;
}

std::unique_ptr<AST::Expr> Parser::expression()
{
    return term();
}

std::unique_ptr<AST::Expr> Parser::term()
{
    auto expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS}))
    {
        Token op = previous();
        auto right = factor();
        auto new_expr = std::make_unique<AST::BinaryExpr>();
        new_expr->left = std::move(expr);
        new_expr->op = op;
        new_expr->right = std::move(right);
        expr = std::move(new_expr);
    }

    return expr;
}

std::unique_ptr<AST::Expr> Parser::factor()
{
    auto expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
    {
        Token op = previous();
        auto right = unary();
        auto new_expr = std::make_unique<AST::BinaryExpr>();
        new_expr->left = std::move(expr);
        new_expr->op = op;
        new_expr->right = std::move(right);
        expr = std::move(new_expr);
    }

    return expr;
}

std::unique_ptr<AST::Expr> Parser::unary()
{
    if (match({TokenType::MINUS}))
    {
        Token op = previous();
        auto right = unary();
        auto expr = std::make_unique<AST::UnaryExpr>();
        expr->op = op;
        expr->right = std::move(right);
        return expr;
    }

    return primary();
}

std::unique_ptr<AST::Expr> Parser::primary()
{
    switch (peek().type)
    {
    case TokenType::TRUE:
    {
        advance();
        auto bool_expr = std::make_unique<AST::BoolExpr>();
        bool_expr->value = true;
        return bool_expr;
    }
    case TokenType::FALSE:
    {
        advance();
        auto bool_expr = std::make_unique<AST::BoolExpr>();
        bool_expr->value = false;
        return bool_expr;
    }
    case TokenType::STRING:
    case TokenType::NUMBER:
    {
        advance();
        auto literal = std::make_unique<AST::LiteralExpr>();
        literal->value = previous();
        return literal;
    }
    case TokenType::HEX_COLOR:
    {
        advance();
        auto color_expr = std::make_unique<AST::ColorExpr>();
        std::string hex_str = previous().lexeme.substr(1); // remove '#'
        if (hex_str.length() != 6)
        {
            Token error_token = previous();
            std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                        std::to_string(error_token.column) +
                                        ": Invalid hex color format. Must be 6 hex digits.";
            throw std::runtime_error(error_message);
        }
        try
        {
            unsigned long value = std::stoul(hex_str, nullptr, 16);
            color_expr->r = (value >> 16) & 0xFF;
            color_expr->g = (value >> 8) & 0xFF;
            color_expr->b = value & 0xFF;
        }
        catch (const std::invalid_argument &e)
        {
            Token error_token = previous();
            std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                        std::to_string(error_token.column) + ": Invalid hex color value.";
            throw std::runtime_error(error_message);
        }
        catch (const std::out_of_range &e)
        {
            Token error_token = previous();
            std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                        std::to_string(error_token.column) + ": Hex color value out of range.";
            throw std::runtime_error(error_message);
        }
        return color_expr;
    }
    case TokenType::LEFT_BRACKET:
    {
        advance();
        return array();
    }
    case TokenType::LEFT_PAREN:
    {
        advance();
        auto expr = expression();
        if (match({TokenType::COMMA}))
        {
            auto set_expr = std::make_unique<AST::SetExpr>();
            set_expr->elements.push_back(std::move(expr));
            if (!check(TokenType::RIGHT_PAREN))
            {
                do
                {
                    set_expr->elements.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after set elements.");
            return set_expr;
        }
        else
        {
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            auto grouping_expr = std::make_unique<AST::GroupingExpr>();
            grouping_expr->expression = std::move(expr);
            return grouping_expr;
        }
    }
    case TokenType::LEFT_BRACE:
    {
        advance();
        return map();
    }
    case TokenType::DYNA:
    {
        advance();
        return dyna();
    }
    case TokenType::COLOR:
    {
        advance();
        return color();
    }
    case TokenType::COORD:
    {
        advance();
        return coord();
    }
    case TokenType::PATH:
    {
        advance();
        return path();
    }
    case TokenType::LIST:
    {
        advance();
        return list();
    }
    case TokenType::ARRAY:
    {
        advance();
        return array_func();
    }
    case TokenType::AT:
    {
        advance();
        if (match({TokenType::LEFT_BRACE}))
        {
            Token section = consume(TokenType::IDENTIFIER, "Expect section name for cross-reference.");
            consume(TokenType::DOT, "Expect '.' between section and key.");
            Token key = consume(TokenType::IDENTIFIER, "Expect key for cross-reference.");
            consume(TokenType::RIGHT_BRACE, "Expect '}' to close cross-reference.");
            auto ref = std::make_unique<AST::CrossSectionRefExpr>();
            ref->section = section;
            ref->key = key;
            return ref;
        }
        else
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect macro name after '@'.");
            auto macro = std::make_unique<AST::MacroExpr>();
            macro->name = name;
            return macro;
        }
    }
    case TokenType::DOLLAR:
    {
        advance();
        consume(TokenType::LEFT_BRACE, "Expect '{' after '$'.");
        Token name = consume(TokenType::IDENTIFIER, "Expect environment variable name.");
        consume(TokenType::RIGHT_BRACE, "Expect '}' to close environment variable reference.");
        auto ref = std::make_unique<AST::EnvVarRefExpr>();
        ref->name = name;
        return ref;
    }
    default:
    {
        Token error_token = peek();
        std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                    std::to_string(error_token.column) + ": Expect expression.";
        throw std::runtime_error(error_message);
    }
    }
}

std::unique_ptr<AST::Expr> Parser::array()
{
    auto array_expr = std::make_unique<AST::ArrayExpr>();
    if (!check(TokenType::RIGHT_BRACKET))
    {
        do
        {
            array_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return array_expr;
}

std::unique_ptr<AST::Expr> Parser::set()
{
    auto set_expr = std::make_unique<AST::SetExpr>();
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            set_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after set elements.");
    return set_expr;
}

std::unique_ptr<AST::Expr> Parser::map()
{
    std::vector<std::pair<Token, std::unique_ptr<AST::Expr>>> elements;
    bool trailing_comma = false;

    if (!check(TokenType::RIGHT_BRACE))
    {
        do
        {
            Token key = consume(TokenType::IDENTIFIER, "Expect map key.");
            consume(TokenType::COLON, "Expect ':' after map key.");
            std::unique_ptr<AST::Expr> value = expression();
            elements.emplace_back(key, std::move(value));
        } while (match({TokenType::COMMA}));
    }

    // After the loop, check if the last token was a comma
    if (previous().type == TokenType::COMMA)
    {
        trailing_comma = true;
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after map elements.");

    // Now, decide which AST node to create
    if (elements.size() == 1 && !trailing_comma)
    {
        auto struct_expr = std::make_unique<AST::StructExpr>();
        struct_expr->key = elements[0].first;
        struct_expr->value = std::move(elements[0].second);
        return struct_expr;
    }
    else
    {
        auto map_expr = std::make_unique<AST::MapExpr>();
        map_expr->elements = std::move(elements);
        return map_expr;
    }
}

std::unique_ptr<AST::Expr> Parser::color()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'color' keyword.");
    Token r = consume(TokenType::NUMBER, "Expect red color component.");
    consume(TokenType::COMMA, "Expect ',' after red component.");
    Token g = consume(TokenType::NUMBER, "Expect green color component.");
    consume(TokenType::COMMA, "Expect ',' after green component.");
    Token b = consume(TokenType::NUMBER, "Expect blue color component.");
    consume(TokenType::RIGHT_PAREN, "Expect ')' after color components.");

    auto color_expr = std::make_unique<AST::ColorExpr>();
    color_expr->r = static_cast<uint8_t>(std::get<double>(r.literal));
    color_expr->g = static_cast<uint8_t>(std::get<double>(g.literal));
    color_expr->b = static_cast<uint8_t>(std::get<double>(b.literal));
    return color_expr;
}

std::unique_ptr<AST::Expr> Parser::coord()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'coord' keyword.");
    auto coord_expr = std::make_unique<AST::CoordExpr>();
    coord_expr->x = expression();
    consume(TokenType::COMMA, "Expect ',' after x coordinate.");
    coord_expr->y = expression();
    if (match({TokenType::COMMA}))
    {
        coord_expr->z = expression();
    }
    else
    {
        coord_expr->z = nullptr;
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after coordinate values.");
    return coord_expr;
}

std::unique_ptr<AST::Expr> Parser::dyna()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'Dyna' keyword.");
    auto expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after Dyna expression.");
    auto dyna_expr = std::make_unique<AST::DynaExpr>();
    dyna_expr->expression = std::move(expr);
    return dyna_expr;
}

std::unique_ptr<AST::Expr> Parser::path()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'path' keyword.");
    auto path_expr = std::make_unique<AST::PathExpr>();
    if (peek().type == TokenType::STRING)
    {
        path_expr->path = std::get<std::string>(consume(TokenType::STRING, "Expect path string.").literal);
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after path.");
    return path_expr;
}

std::unique_ptr<AST::Expr> Parser::list()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'list' keyword.");
    auto list_expr = std::make_unique<AST::ListExpr>();
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            list_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after list elements.");
    return list_expr;
}

std::unique_ptr<AST::Expr> Parser::array_func()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'array' keyword.");
    auto array_expr = std::make_unique<AST::ArrayExpr>();
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            array_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after array elements.");
    return array_expr;
}

bool Parser::match(const std::vector<TokenType> &types)
{
    for (TokenType type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &message)
{
    if (check(type))
        return advance();

    Token error_token = peek();
    std::string error_message = "Error at line " + std::to_string(error_token.line) + ", column " +
                                std::to_string(error_token.column) + ": " + message;
    throw std::runtime_error(error_message);
}

bool Parser::check(TokenType type)
{
    if (is_at_end())
        return false;
    return peek().type == type;
}

Token Parser::advance()
{
    if (!is_at_end())
        m_current++;
    return previous();
}

bool Parser::is_at_end()
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek()
{
    return m_tokens[m_current];
}

Token Parser::peek_next()
{
    if (m_current + 1 >= m_tokens.size())
        return m_tokens.back();
    return m_tokens[m_current + 1];
}

Token Parser::previous()
{
    return m_tokens[m_current - 1];
}

} // namespace YINI
