#include "Parser.h"
#include <map>

namespace Yini
{
    // Operator precedence mapping for the Pratt parser
    enum Precedence
    {
        LOWEST = 0,
        EQUALS,      // =
        SUM,         // + or -
        PRODUCT,     // * or /
        PREFIX,      // -X or !X
        CALL         // myFunction(X)
    };

    static std::map<TokenType, Precedence> precedences = {
        {TokenType::Assign, EQUALS},
        {TokenType::Plus, SUM},
        {TokenType::Minus, SUM},
        {TokenType::Star, PRODUCT},
        {TokenType::Slash, PRODUCT},
        {TokenType::Percent, PRODUCT},
        {TokenType::LParen, CALL}
    };

    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        // Prime the tokens
        nextToken();
        nextToken();
    }

    void Parser::nextToken()
    {
        m_currentToken = m_peekToken;
        m_peekToken = m_lexer.nextToken();
    }

    const std::vector<std::string>& Parser::getErrors() const
    {
        return m_errors;
    }

    void Parser::peekError(TokenType expected)
    {
        std::string msg = "Error: Expected next token to be " + std::to_string((int)expected) +
                          ", got " + std::to_string((int)m_peekToken.type) + " instead.";
        m_errors.push_back(msg);
    }

    int Parser::peekPrecedence()
    {
        if (precedences.count(m_peekToken.type))
        {
            return precedences[m_peekToken.type];
        }
        return LOWEST;
    }

    int Parser::currentPrecedence()
    {
        if (precedences.count(m_currentToken.type))
        {
            return precedences[m_currentToken.type];
        }
        return LOWEST;
    }

    std::unique_ptr<Ast::YiniDocument> Parser::parseDocument()
    {
        auto doc = std::make_unique<Ast::YiniDocument>();
        while (m_currentToken.type != TokenType::Eof)
        {
            auto stmt = parseStatement();
            if (stmt)
            {
                doc->statements.push_back(std::move(stmt));
            }
            // parseStatement should consume the tokens it processes.
            // If it doesn't (e.g., on an error), we must advance to avoid an infinite loop.
            // The current logic in parseSection handles advancing, so we may not need this.
            // Let's remove the extra nextToken() from here.
        }
        return doc;
    }

    std::unique_ptr<Ast::Statement> Parser::parseStatement()
    {
        if (m_currentToken.type == TokenType::LBracket)
        {
            return parseSection();
        }
        else if (m_currentToken.type == TokenType::Identifier && m_peekToken.type == TokenType::Assign)
        {
            auto stmt = parseKeyValuePair();
            nextToken(); // Consume the last token of the expression
            return stmt;
        }
        else if (m_currentToken.type == TokenType::PlusAssign)
        {
            auto stmt = parseQuickRegister();
            nextToken(); // Consume the last token of the expression
            return stmt;
        }

        // If we see a token we don't recognize as a statement, advance past it to avoid infinite loops.
        nextToken();
        return nullptr;
    }

    std::unique_ptr<Ast::Section> Parser::parseSection()
    {
        auto section = std::make_unique<Ast::Section>();
        section->token = m_currentToken; // The '[' token

        // We expect an identifier or a hash for special sections
        if (m_peekToken.type != TokenType::Identifier && m_peekToken.type != TokenType::Hash)
        {
            return nullptr;
        }
        nextToken(); // Consume '['

        bool is_special = false;
        if (m_currentToken.type == TokenType::Hash)
        {
            is_special = true;
            nextToken(); // Consume '#'
        }

        section->name = std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal);
        if (is_special)
        {
            section->name->value = "#" + section->name->value;
        }
        nextToken(); // Consume identifier

        // TODO: Parse inheritance here

        if (m_currentToken.type != TokenType::RBracket)
        {
            return nullptr; // Missing ']'
        }
        nextToken(); // Consume ']'

        // Parse all statements belonging to this section
        while (m_currentToken.type != TokenType::LBracket && m_currentToken.type != TokenType::Eof)
        {
            auto stmt = parseStatement();
            if (stmt)
            {
                section->statements.push_back(std::move(stmt));
            }
        }

        return section;
    }

    std::unique_ptr<Ast::KeyValuePair> Parser::parseKeyValuePair()
    {
        auto kvp = std::make_unique<Ast::KeyValuePair>();
        kvp->key = std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal);

        nextToken(); // Consume key identifier, m_currentToken is now '='
        kvp->token = m_currentToken;
        nextToken(); // Consume '='

        kvp->value = parseExpression(LOWEST);

        return kvp;
    }

    std::unique_ptr<Ast::QuickRegister> Parser::parseQuickRegister()
    {
        auto qr = std::make_unique<Ast::QuickRegister>();
        qr->token = m_currentToken; // The '+=' token
        nextToken(); // Consume '+='

        qr->value = parseExpression(LOWEST);

        return qr;
    }

    std::unique_ptr<Ast::Expression> Parser::parseExpression(int precedence)
    {
        auto leftExp = parsePrefixExpression();

        while (m_peekToken.type != TokenType::Eof && precedence < peekPrecedence())
        {
            nextToken();
            leftExp = parseInfixExpression(std::move(leftExp));
        }

        return leftExp;
    }

    std::unique_ptr<Ast::Expression> Parser::parsePrefixExpression()
    {
        switch (m_currentToken.type)
        {
            case TokenType::Identifier:
                return parseIdentifier();
            case TokenType::Integer:
                return parseIntegerLiteral();
            case TokenType::Float:
                return parseFloatLiteral();
            case TokenType::True:
            case TokenType::False:
                return parseBooleanLiteral();
            case TokenType::String:
                 return parseStringLiteral();
            default:
                return nullptr;
        }
    }

    std::unique_ptr<Ast::Expression> Parser::parseInfixExpression(std::unique_ptr<Ast::Expression> left)
    {
        auto infix = std::make_unique<Ast::InfixExpression>();
        infix->token = m_currentToken;
        infix->op = m_currentToken.literal;
        infix->left = std::move(left);

        int precedence = currentPrecedence();
        nextToken();
        infix->right = parseExpression(precedence);

        return infix;
    }

    std::unique_ptr<Ast::Identifier> Parser::parseIdentifier()
    {
        return std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal);
    }

    std::unique_ptr<Ast::Expression> Parser::parseIntegerLiteral()
    {
        auto lit = std::make_unique<Ast::IntegerLiteral>();
        lit->token = m_currentToken;
        try {
            lit->value = std::stoll(m_currentToken.literal);
        } catch (const std::invalid_argument&) {
            m_errors.push_back("Could not parse integer: " + m_currentToken.literal);
            return nullptr;
        }
        return lit;
    }

    std::unique_ptr<Ast::Expression> Parser::parseFloatLiteral()
    {
        auto lit = std::make_unique<Ast::FloatLiteral>();
        lit->token = m_currentToken;
        try {
            lit->value = std::stod(m_currentToken.literal);
        } catch (const std::invalid_argument&) {
            m_errors.push_back("Could not parse float: " + m_currentToken.literal);
            return nullptr;
        }
        return lit;
    }

    std::unique_ptr<Ast::Expression> Parser::parseBooleanLiteral()
    {
        auto lit = std::make_unique<Ast::BooleanLiteral>();
        lit->token = m_currentToken;
        lit->value = (m_currentToken.type == TokenType::True);
        return lit;
    }

    std::unique_ptr<Ast::Expression> Parser::parseStringLiteral()
    {
        auto lit = std::make_unique<Ast::StringLiteral>();
        lit->token = m_currentToken;
        lit->value = m_currentToken.literal;
        return lit;
    }
}
