#include "Parser.h"
#include <stdexcept>

namespace YINI
{

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens)
{
}

std::vector<std::unique_ptr<AST::Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<AST::Stmt>> statements;
    while (!is_at_end())
    {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<AST::Stmt> Parser::declaration()
{
    if (match({TokenType::LEFT_BRACKET})) {
        if (match({TokenType::HASH})) {
            Token keyword = consume(TokenType::IDENTIFIER, "Expect 'define' or 'include' keyword.");
            if (keyword.lexeme == "define") {
                return define_section_declaration();
            } else if (keyword.lexeme == "include") {
                return include_section_declaration();
            } else {
                throw std::runtime_error("Unknown directive '#" + keyword.lexeme + "'.");
            }
        }
        return section_declaration();
    }
    return key_value_statement();
}

std::unique_ptr<AST::Stmt> Parser::section_declaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect section name.");

    auto section = std::make_unique<AST::SectionStmt>();
    section->name = name;

    if (match({TokenType::COLON}))
    {
        do {
            section->parent_sections.push_back(consume(TokenType::IDENTIFIER, "Expect parent section name."));
        } while (match({TokenType::COMMA}));
    }

    consume(TokenType::RIGHT_BRACKET, "Expect ']' after section declaration.");

    while (!check(TokenType::LEFT_BRACKET) && !is_at_end())
    {
        section->statements.push_back(key_value_statement());
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

std::unique_ptr<AST::KeyValueStmt> Parser::key_value_statement()
{
    Token key = consume(TokenType::IDENTIFIER, "Expect key.");
    consume(TokenType::EQUAL, "Expect '=' after key.");
    auto value = expression();

    auto stmt = std::make_unique<AST::KeyValueStmt>();
    stmt->key = key;
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
    auto expr = primary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
    {
        Token op = previous();
        auto right = primary();
        auto new_expr = std::make_unique<AST::BinaryExpr>();
        new_expr->left = std::move(expr);
        new_expr->op = op;
        new_expr->right = std::move(right);
        expr = std::move(new_expr);
    }

    return expr;
}

std::unique_ptr<AST::Expr> Parser::primary()
{
    if (match({TokenType::TRUE})) {
        auto bool_expr = std::make_unique<AST::BoolExpr>();
        bool_expr->value = true;
        return bool_expr;
    }

    if (match({TokenType::FALSE})) {
        auto bool_expr = std::make_unique<AST::BoolExpr>();
        bool_expr->value = false;
        return bool_expr;
    }

    if (match({TokenType::STRING, TokenType::NUMBER}))
    {
        auto literal = std::make_unique<AST::LiteralExpr>();
        literal->value = previous();
        return literal;
    }

    if (match({TokenType::HEX_COLOR})) {
        auto color_expr = std::make_unique<AST::ColorExpr>();
        std::string hex_str = previous().lexeme.substr(1); // remove '#'
        if (hex_str.length() != 6) {
            throw std::runtime_error("Invalid hex color format. Must be 6 hex digits.");
        }
        try {
            unsigned long value = std::stoul(hex_str, nullptr, 16);
            color_expr->r = (value >> 16) & 0xFF;
            color_expr->g = (value >> 8) & 0xFF;
            color_expr->b = value & 0xFF;
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid hex color value.");
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Hex color value out of range.");
        }
        return color_expr;
    }

    if (match({TokenType::LEFT_BRACKET})) {
        return array();
    }

    if (match({TokenType::LEFT_PAREN})) {
        auto expr = expression();
        if (match({TokenType::COMMA})) {
            auto set_expr = std::make_unique<AST::SetExpr>();
            set_expr->elements.push_back(std::move(expr));
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    set_expr->elements.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after set elements.");
            return set_expr;
        } else {
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            auto grouping_expr = std::make_unique<AST::GroupingExpr>();
            grouping_expr->expression = std::move(expr);
            return grouping_expr;
        }
    }

    if (match({TokenType::LEFT_BRACE})) {
        return map();
    }

    if (match({TokenType::COLOR})) {
        return color();
    }

    if (match({TokenType::COORD})) {
        return coord();
    }

    if (match({TokenType::AT})) {
        if (match({TokenType::LEFT_BRACE})) {
            Token section = consume(TokenType::IDENTIFIER, "Expect section name for cross-reference.");
            consume(TokenType::DOT, "Expect '.' between section and key.");
            Token key = consume(TokenType::IDENTIFIER, "Expect key for cross-reference.");
            consume(TokenType::RIGHT_BRACE, "Expect '}' to close cross-reference.");
            auto ref = std::make_unique<AST::CrossSectionRefExpr>();
            ref->section = section;
            ref->key = key;
            return ref;
        } else {
            Token name = consume(TokenType::IDENTIFIER, "Expect macro name after '@'.");
            auto macro = std::make_unique<AST::MacroExpr>();
            macro->name = name;
            return macro;
        }
    }

    if (match({TokenType::DOLLAR})) {
        consume(TokenType::LEFT_BRACE, "Expect '{' after '$'.");
        Token name = consume(TokenType::IDENTIFIER, "Expect environment variable name.");
        consume(TokenType::RIGHT_BRACE, "Expect '}' to close environment variable reference.");
        auto ref = std::make_unique<AST::EnvVarRefExpr>();
        ref->name = name;
        return ref;
    }

    // For now, we'll just throw an error for unsupported expressions.
    throw std::runtime_error("Expect expression.");
}

std::unique_ptr<AST::Expr> Parser::array() {
    auto array_expr = std::make_unique<AST::ArrayExpr>();
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            array_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return array_expr;
}

std::unique_ptr<AST::Expr> Parser::set() {
    auto set_expr = std::make_unique<AST::SetExpr>();
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            set_expr->elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after set elements.");
    return set_expr;
}

std::unique_ptr<AST::Expr> Parser::map() {
    auto map_expr = std::make_unique<AST::MapExpr>();
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            Token key = consume(TokenType::IDENTIFIER, "Expect map key.");
            consume(TokenType::COLON, "Expect ':' after map key.");
            std::unique_ptr<AST::Expr> value = expression();
            map_expr->elements.emplace_back(key, std::move(value));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after map elements.");
    return map_expr;
}

std::unique_ptr<AST::Expr> Parser::color() {
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

std::unique_ptr<AST::Expr> Parser::coord() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'coord' keyword.");
    auto coord_expr = std::make_unique<AST::CoordExpr>();
    coord_expr->x = expression();
    consume(TokenType::COMMA, "Expect ',' after x coordinate.");
    coord_expr->y = expression();
    if (match({TokenType::COMMA})) {
        coord_expr->z = expression();
    } else {
        coord_expr->z = nullptr;
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after coordinate values.");
    return coord_expr;
}

bool Parser::match(const std::vector<TokenType>& types)
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

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) return advance();
    throw std::runtime_error(message);
}

bool Parser::check(TokenType type)
{
    if (is_at_end()) return false;
    return peek().type == type;
}

Token Parser::advance()
{
    if (!is_at_end()) m_current++;
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

Token Parser::previous()
{
    return m_tokens[m_current - 1];
}

} // namespace YINI