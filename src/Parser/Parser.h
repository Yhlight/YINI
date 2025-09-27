#pragma once

#include "Lexer/Lexer.h"
#include "Parser/AST.h"
#include <vector>
#include <string>
#include <memory>

namespace YINI
{
    // Precedence levels for operators
    enum Precedence {
        LOWEST,
        SUM,        // +
        PRODUCT,    // *
        PREFIX      // -X or !X
    };

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
        std::unique_ptr<QuickRegisterStatement> parseQuickRegisterStatement();

        // Expression parsing
        std::unique_ptr<Expression> parseExpression(Precedence precedence);
        std::unique_ptr<Expression> parsePrefixExpression();
        std::unique_ptr<Expression> parseInfixExpression(std::unique_ptr<Expression> left);
        std::unique_ptr<Expression> parseStringLiteral();
        std::unique_ptr<Expression> parseIntegerLiteral();
        std::unique_ptr<Expression> parseFloatLiteral();
        std::unique_ptr<Expression> parseBooleanLiteral();
        std::unique_ptr<Expression> parseGroupedExpression();
        std::unique_ptr<Expression> parsePrefixOperatorExpression();

        bool expectPeek(TokenType t);
        Precedence peekPrecedence();

        Lexer& m_lexer;
        Token m_currentToken;
        Token m_peekToken;
        std::vector<std::string> m_errors;
    };
}