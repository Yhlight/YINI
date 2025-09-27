#pragma once

#include "Lexer/Lexer.h"
#include "Parser/AST.h"
#include <vector>
#include <string>

namespace YINI
{
    class Parser
    {
    public:
        Parser(Lexer& lexer);

        std::unique_ptr<Program> parseProgram();
        const std::vector<std::string>& getErrors() const;

    private:
        void nextToken();
        std::unique_ptr<Statement> parseStatement();
        std::unique_ptr<SectionStatement> parseSectionStatement();
        std::unique_ptr<KeyValuePairStatement> parseKeyValuePairStatement();

        bool expectPeek(TokenType t);

        Lexer& m_lexer;
        Token m_currentToken;
        Token m_peekToken;
        std::vector<std::string> m_errors;
    };
}