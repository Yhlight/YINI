#pragma once

#include "Lexer.h"
#include "Ast.h"
#include "Token.h"
#include <vector>
#include <string>
#include <map>

namespace YINI
{
    enum Precedence {
        LOWEST,
        EQUALS,      // =
        SUM,         // +
        PRODUCT,     // *
        PREFIX,      // -X or !X
        CALL,        // myFunction(X)
        INDEX        // array[index]
    };

    class Parser
    {
    public:
        Parser(Lexer& lexer);

        std::unique_ptr<Program> ParseProgram();
        const std::vector<std::string>& Errors() const { return m_errors; }

    private:
        using prefixParseFn = std::shared_ptr<Expression> (Parser::*)();
        using infixParseFn = std::shared_ptr<Expression> (Parser::*)(std::shared_ptr<Expression>);

        void nextToken();
        void registerPrefix(TokenType type, prefixParseFn fn);
        void registerInfix(TokenType type, infixParseFn fn);

        std::shared_ptr<Statement> parseStatement();
        std::shared_ptr<Section> parseSectionStatement();
        std::shared_ptr<DefineStatement> parseDefineStatement();
        std::shared_ptr<KeyValuePair> parseKeyValuePair();
        std::shared_ptr<Expression> parseExpression(Precedence precedence);
        std::shared_ptr<Expression> parseIdentifier();
        std::shared_ptr<Expression> parseIntegerLiteral();
        std::shared_ptr<Expression> parseFloatLiteral();
        std::shared_ptr<Expression> parseBooleanLiteral();
        std::shared_ptr<Expression> parseStringLiteral();
        std::shared_ptr<Expression> parseArrayLiteral();
        std::shared_ptr<Expression> parseMacroReference();
        std::vector<std::shared_ptr<Expression>> parseExpressionList(TokenType end);
        std::shared_ptr<Expression> parseInfixExpression(std::shared_ptr<Expression> left);

        Precedence peekPrecedence();
        Precedence curPrecedence();
        bool expectPeek(TokenType t);
        bool currentTokenIs(TokenType t);
        bool peekTokenIs(TokenType t);

        Lexer& m_lexer;
        Token m_currentToken;
        Token m_peekToken;
        std::vector<std::string> m_errors;
        std::map<TokenType, prefixParseFn> m_prefixParseFns;
        std::map<TokenType, infixParseFn> m_infixParseFns;
    };
}