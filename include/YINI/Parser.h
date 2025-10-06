#pragma once

#include "Lexer.h"
#include "Ast.h"
#include "Token.h"

#include <vector>
#include <string>

namespace YINI
{
    class Parser
    {
    public:
        Parser(Lexer& lexer);

        std::unique_ptr<Program> ParseProgram();
        const std::vector<std::string>& Errors() const { return m_errors; }

    private:
        void nextToken();

        std::shared_ptr<Statement> parseStatement();
        std::shared_ptr<Section> parseSectionStatement();
        std::shared_ptr<KeyValuePair> parseKeyValuePair();

        bool expectPeek(TokenType t);
        bool currentTokenIs(TokenType t);
        bool peekTokenIs(TokenType t);

        Lexer& m_lexer;
        Token m_currentToken;
        Token m_peekToken;
        std::vector<std::string> m_errors;
    };
}