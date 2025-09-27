#include "Parser/Parser.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <map>

namespace YINI
{
    std::map<TokenType, Precedence> precedences = {
        {TokenType::Plus,     SUM},
        {TokenType::Minus,    SUM},
        {TokenType::Slash,    PRODUCT},
        {TokenType::Asterisk, PRODUCT},
    };

    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        // Read two tokens, so m_currentToken and m_peekToken are both set
        nextToken();
        nextToken();
    }

    void Parser::nextToken()
    {
        m_currentToken = m_peekToken;
        m_peekToken = m_lexer.nextToken();
    }

    Precedence Parser::peekPrecedence() {
        if (precedences.count(m_peekToken.type)) {
            return precedences[m_peekToken.type];
        }
        return LOWEST;
    }

    bool Parser::expectPeek(TokenType t) {
        if (m_peekToken.type == t) {
            nextToken();
            return true;
        } else {
            // Here you would typically add an error to the m_errors vector
            return false;
        }
    }

    std::unique_ptr<SectionStatement> Parser::parseSectionStatement() {
        auto stmt = std::make_unique<SectionStatement>();
        stmt->token = m_currentToken;

        if (!expectPeek(TokenType::Identifier)) {
            return nullptr;
        }

        stmt->name = m_currentToken.literal;

        if (!expectPeek(TokenType::RBracket)) {
            return nullptr;
        }

        return stmt;
    }

    std::unique_ptr<Expression> Parser::parseStringLiteral() {
        auto literal = std::make_unique<StringLiteral>();
        literal->token = m_currentToken;
        literal->value = m_currentToken.literal;
        return literal;
    }

    std::unique_ptr<Expression> Parser::parseIntegerLiteral() {
        auto literal = std::make_unique<IntegerLiteral>();
        literal->token = m_currentToken;
        try {
            literal->value = std::stoll(m_currentToken.literal);
        } catch (const std::invalid_argument& ia) {
            return nullptr;
        } catch (const std::out_of_range& oor) {
            return nullptr;
        }
        return literal;
    }

    std::unique_ptr<Expression> Parser::parseFloatLiteral() {
        auto literal = std::make_unique<FloatLiteral>();
        literal->token = m_currentToken;
        try {
            literal->value = std::stod(m_currentToken.literal);
        } catch (const std::invalid_argument& ia) {
            return nullptr;
        } catch (const std::out_of_range& oor) {
            return nullptr;
        }
        return literal;
    }

    std::unique_ptr<Expression> Parser::parseBooleanLiteral() {
        auto literal = std::make_unique<BooleanLiteral>();
        literal->token = m_currentToken;
        literal->value = (m_currentToken.type == TokenType::True);
        return literal;
    }

    std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left) {
        auto expr = std::make_unique<InfixExpression>();
        expr->left = std::move(left);
        expr->operator_token = m_currentToken;

        Precedence precedence = precedences.count(m_currentToken.type) ? precedences[m_currentToken.type] : LOWEST;
        nextToken();
        expr->right = parseExpression(precedence);

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseGroupedExpression() {
        nextToken(); // Consume '('
        auto expr = parseExpression(LOWEST);
        if (!expectPeek(TokenType::RParen)) {
            return nullptr;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parsePrefixOperatorExpression() {
        auto expr = std::make_unique<PrefixExpression>();
        expr->operator_token = m_currentToken;
        nextToken(); // Move past the operator
        expr->right = parseExpression(PREFIX);
        return expr;
    }

    std::unique_ptr<Expression> Parser::parsePrefixExpression() {
        switch (m_currentToken.type) {
            case TokenType::String:
                return parseStringLiteral();
            case TokenType::Integer:
                return parseIntegerLiteral();
            case TokenType::Float:
                return parseFloatLiteral();
            case TokenType::True:
            case TokenType::False:
                return parseBooleanLiteral();
            case TokenType::LParen:
                return parseGroupedExpression();
            case TokenType::Minus:
                return parsePrefixOperatorExpression();
            default:
                return nullptr;
        }
    }

    std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
        auto left_expr = parsePrefixExpression();
        if (left_expr == nullptr) {
            return nullptr;
        }

        while (m_peekToken.type != TokenType::EndOfFile && precedence < peekPrecedence()) {
            nextToken(); // Move to the infix operator
            left_expr = parseInfixExpression(std::move(left_expr));
        }

        return left_expr;
    }

    std::unique_ptr<KeyValuePairStatement> Parser::parseKeyValuePairStatement() {
        auto stmt = std::make_unique<KeyValuePairStatement>();
        stmt->token = m_currentToken; // The identifier (key)

        if (!expectPeek(TokenType::Assign)) {
            return nullptr;
        }

        nextToken(); // Move to the start of the expression

        stmt->value = parseExpression(LOWEST);

        return stmt;
    }

    std::unique_ptr<QuickRegisterStatement> Parser::parseQuickRegisterStatement() {
        auto stmt = std::make_unique<QuickRegisterStatement>();
        stmt->token = m_currentToken; // The '+=' token

        nextToken(); // Move to the value expression

        stmt->value = parseExpression(LOWEST);

        return stmt;
    }

    std::unique_ptr<Statement> Parser::parseStatement() {
        switch (m_currentToken.type) {
            case TokenType::LBracket:
                return parseSectionStatement();
            case TokenType::Identifier:
                return parseKeyValuePairStatement();
            case TokenType::PlusAssign:
                return parseQuickRegisterStatement();
            default:
                return nullptr;
        }
    }

    std::unique_ptr<Program> Parser::parseProgram()
    {
        auto program = std::make_unique<Program>();

        while (m_currentToken.type != TokenType::EndOfFile) {
            auto stmt = parseStatement();
            if (stmt != nullptr) {
                program->statements.push_back(std::move(stmt));
            }
            nextToken();
        }
        return program;
    }

    const std::vector<std::string>& Parser::getErrors() const
    {
        return m_errors;
    }
}