#include "YINI/Parser.h"
#include "YINI/Ast.h"

namespace YINI
{
    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        // Initialize with the first two tokens
        nextToken();
        nextToken();
    }

    void Parser::nextToken()
    {
        m_currentToken = m_peekToken;
        m_peekToken = m_lexer.NextToken();
    }

    std::unique_ptr<Program> Parser::ParseProgram()
    {
        auto program = std::make_unique<Program>();

        while (!currentTokenIs(TokenType::Eof))
        {
            auto stmt = parseStatement();
            if (stmt)
            {
                program->statements.push_back(stmt);
            }
            nextToken();
        }

        return program;
    }

    std::shared_ptr<Statement> Parser::parseStatement()
    {
        switch (m_currentToken.type)
        {
            case TokenType::Section:
                return parseSectionStatement();
            // In the future, other statement types would be handled here
            default:
                return nullptr;
        }
    }

    std::shared_ptr<Section> Parser::parseSectionStatement()
    {
        auto section = std::make_shared<Section>();
        section->name = m_currentToken.literal;

        nextToken(); // Consume the section token

        // Parse key-value pairs until the next section or EOF
        while (!currentTokenIs(TokenType::Eof) && !currentTokenIs(TokenType::Section) && !currentTokenIs(TokenType::Define) && !currentTokenIs(TokenType::Include) && !currentTokenIs(TokenType::Schema))
        {
            // Skip comments and other non-key-value tokens for now
            if (currentTokenIs(TokenType::Identifier)) {
                 auto pair = parseKeyValuePair();
                 if (pair) {
                     section->pairs.push_back(pair);
                 }
            }
            // If we don't advance the token, we'll have an infinite loop
            if (currentTokenIs(TokenType::Identifier)) continue;

            nextToken();
        }

        return section;
    }

    std::shared_ptr<KeyValuePair> Parser::parseKeyValuePair()
    {
        auto pair = std::make_shared<KeyValuePair>();
        pair->key = m_currentToken.literal;

        if (!expectPeek(TokenType::Assign)) {
            // Error: expected '='
            return nullptr;
        }
        nextToken(); // Consume '='

        // For now, we are just storing the literal of the value token.
        // In the future, this would parse a complex expression.
        pair->value = m_currentToken.literal;

        // No need to call nextToken() here, the main loop does it.

        return pair;
    }


    bool Parser::currentTokenIs(TokenType t)
    {
        return m_currentToken.type == t;
    }

    bool Parser::peekTokenIs(TokenType t)
    {
        return m_peekToken.type == t;
    }

    bool Parser::expectPeek(TokenType t)
    {
        if (peekTokenIs(t)) {
            nextToken();
            return true;
        } else {
            // In a real implementation, we'd add an error here
            return false;
        }
    }
}