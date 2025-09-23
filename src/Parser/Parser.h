#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include "../Lexer/Lexer.h"
#include "Ast.h"
#include "../Common/Error.h"
#include <vector>
#include <string>
#include <memory>

namespace Yini
{
    class Parser
    {
    public:
        explicit Parser(Lexer& lexer);

        std::unique_ptr<Ast::YiniDocument> parseDocument();

        const std::vector<YiniError>& getErrors() const;

    private:
        void nextToken();
        bool currentTokenIs(TokenType t) const;
        bool peekTokenIs(TokenType t) const;
        bool expectPeek(TokenType t);

        std::unique_ptr<Ast::Statement> parseStatement();
        std::unique_ptr<Ast::Section> parseSection();
        std::unique_ptr<Ast::KeyValuePair> parseKeyValuePair();
        std::unique_ptr<Ast::QuickRegister> parseQuickRegister();

        // Expression parsing
        std::unique_ptr<Ast::Expression> parseExpression(int precedence);
        std::unique_ptr<Ast::Expression> parsePrefixExpression();
        std::unique_ptr<Ast::Expression> parseInfixExpression(std::unique_ptr<Ast::Expression> left);

        // Prefix parsers
        std::unique_ptr<Ast::Identifier> parseIdentifier();
        std::unique_ptr<Ast::Expression> parseIntegerLiteral();
        std::unique_ptr<Ast::Expression> parseFloatLiteral();
        std::unique_ptr<Ast::Expression> parseBooleanLiteral();
        std::unique_ptr<Ast::Expression> parseStringLiteral();
        std::unique_ptr<Ast::Expression> parseMacroReference();
        std::unique_ptr<Ast::Expression> parseArrayLiteral();
        std::unique_ptr<Ast::Expression> parseMapLiteral();
        std::unique_ptr<Ast::Expression> parseGroupedExpression();
        std::unique_ptr<Ast::KeyValueLiteral> parseKeyValueLiteral();

        // Infix parsers
        std::unique_ptr<Ast::Expression> parseFunctionCall(std::unique_ptr<Ast::Expression> function);

        // Helpers
        std::vector<std::unique_ptr<Ast::Expression>> parseExpressionList(TokenType endToken);

        void peekError(TokenType expected);
        int peekPrecedence();
        int currentPrecedence();

        Lexer& m_lexer;
        std::vector<YiniError> m_errors;

        Token m_currentToken;
        Token m_peekToken;
    };
}

#endif // YINI_PARSER_H
