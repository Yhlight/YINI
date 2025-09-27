#include "Parser/Parser.h"
#include <memory>

namespace YINI
{
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

    std::unique_ptr<KeyValuePairStatement> Parser::parseKeyValuePairStatement() {
        auto stmt = std::make_unique<KeyValuePairStatement>();
        stmt->token = m_currentToken; // The identifier (key)

        if (!expectPeek(TokenType::Assign)) {
            return nullptr;
        }

        nextToken(); // Move to the value token

        stmt->value = m_currentToken;

        return stmt;
    }

    std::unique_ptr<Statement> Parser::parseStatement() {
        switch (m_currentToken.type) {
            case TokenType::LBracket:
                return parseSectionStatement();
            case TokenType::Identifier:
                return parseKeyValuePairStatement();
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