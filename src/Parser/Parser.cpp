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
        nextToken();
        nextToken();
    }

    void Parser::nextToken() { m_currentToken = m_peekToken; m_peekToken = m_lexer.nextToken(); }
    bool Parser::currentTokenIs(TokenType t) const { return m_currentToken.type == t; }
    bool Parser::peekTokenIs(TokenType t) const { return m_peekToken.type == t; }

    bool Parser::expectPeek(TokenType t)
    {
        if (peekTokenIs(t)) {
            nextToken();
            return true;
        }
        peekError(t);
        return false;
    }

    const std::vector<YiniError>& Parser::getErrors() const { return m_errors; }

    void Parser::peekError(TokenType expected)
    {
        std::stringstream ss;
        ss << "Expected next token to be " << (int)expected << ", got " << (int)m_peekToken.type << " instead.";
        m_errors.emplace_back(ErrorType::Parsing, ss.str(), m_peekToken.line, m_peekToken.column);
    }

    int Parser::peekPrecedence() { return precedences.count(m_peekToken.type) ? precedences[m_peekToken.type] : LOWEST; }
    int Parser::currentPrecedence() { return precedences.count(m_currentToken.type) ? precedences[m_currentToken.type] : LOWEST; }

    std::unique_ptr<Ast::YiniDocument> Parser::parseDocument()
    {
        auto doc = std::make_unique<Ast::YiniDocument>();
        while (!currentTokenIs(TokenType::Eof))
        {
            auto stmt = parseStatement();
            if (stmt) doc->statements.push_back(std::move(stmt));
            else nextToken(); // On error, advance to avoid infinite loop
        }
        return doc;
    }

    std::unique_ptr<Ast::Statement> Parser::parseStatement()
    {
        if (currentTokenIs(TokenType::LBracket)) return parseSection();
        if (currentTokenIs(TokenType::Identifier) && peekTokenIs(TokenType::Assign)) return parseKeyValuePair();
        if (currentTokenIs(TokenType::PlusAssign)) return parseQuickRegister();
        return nullptr;
    }

    std::unique_ptr<Ast::Section> Parser::parseSection()
    {
        auto section = std::make_unique<Ast::Section>();
        section->token = m_currentToken; // The '[' token

        if (!expectPeek(TokenType::Identifier) && !expectPeek(TokenType::Hash)) return nullptr;

        // Handle special sections like [#define]
        if (currentTokenIs(TokenType::Hash)) {
            if (!expectPeek(TokenType::Identifier)) return nullptr;
            section->name = std::make_unique<Ast::Identifier>(m_currentToken, "#" + m_currentToken.literal);
        } else {
            section->name = std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal);
        }

        // Parse inheritance
        if (peekTokenIs(TokenType::Colon)) {
            nextToken(); // Consume section name identifier
            nextToken(); // Consume ':'

            section->parents.push_back(parseIdentifier());
            while (peekTokenIs(TokenType::Comma)) {
                nextToken(); // Consume identifier
                nextToken(); // Consume ','
                section->parents.push_back(parseIdentifier());
            }
        }

        if (!expectPeek(TokenType::RBracket)) return nullptr;

        // Parse statements inside the section
        while (!peekTokenIs(TokenType::LBracket) && !peekTokenIs(TokenType::Eof)) {
            nextToken();
            auto stmt = parseStatement();
            if (stmt) section->statements.push_back(std::move(stmt));
        }

        return section;
    }

    std::unique_ptr<Ast::KeyValuePair> Parser::parseKeyValuePair()
    {
        auto kvp = std::make_unique<Ast::KeyValuePair>();
        kvp->key = std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal);
        if (!expectPeek(TokenType::Assign)) return nullptr;
        kvp->token = m_currentToken;
        nextToken();
        kvp->value = parseExpression(LOWEST);
        return kvp;
    }

    std::unique_ptr<Ast::QuickRegister> Parser::parseQuickRegister()
    {
        auto qr = std::make_unique<Ast::QuickRegister>();
        qr->token = m_currentToken;
        nextToken();
        qr->value = parseExpression(LOWEST);
        return qr;
    }

    std::unique_ptr<Ast::Expression> Parser::parseExpression(int precedence)
    {
        auto leftExp = parsePrefixExpression();
        while (!peekTokenIs(TokenType::Eof) && precedence < peekPrecedence()) {
            nextToken();
            leftExp = parseInfixExpression(std::move(leftExp));
        }
        return leftExp;
    }

    std::unique_ptr<Ast::Expression> Parser::parsePrefixExpression()
    {
        switch (m_currentToken.type) {
            case TokenType::Identifier: return parseIdentifier();
            case TokenType::Integer: return parseIntegerLiteral();
            case TokenType::Float: return parseFloatLiteral();
            case TokenType::True: case TokenType::False: return parseBooleanLiteral();
            case TokenType::String: return parseStringLiteral();
            case TokenType::At: return parseMacroReference();
            case TokenType::LBracket: return parseArrayLiteral();
            case TokenType::LBrace: if (peekTokenIs(TokenType::LBrace)) return parseMapLiteral(); else return nullptr; // single brace is an error for now
            case TokenType::LParen: return parseGroupedExpression();
            case TokenType::ColorLiteral: { auto cl = std::make_unique<Ast::ColorLiteral>(); cl->token = m_currentToken; return cl; }
            default: return nullptr;
        }
    }

    std::unique_ptr<Ast::Expression> Parser::parseInfixExpression(std::unique_ptr<Ast::Expression> left)
    {
        if (currentTokenIs(TokenType::LParen)) return parseFunctionCall(std::move(left));

        auto infix = std::make_unique<Ast::InfixExpression>();
        infix->token = m_currentToken;
        infix->op = m_currentToken.literal;
        infix->left = std::move(left);
        int precedence = currentPrecedence();
        nextToken();
        infix->right = parseExpression(precedence);
        return infix;
    }

    std::unique_ptr<Ast::Identifier> Parser::parseIdentifier() { return std::make_unique<Ast::Identifier>(m_currentToken, m_currentToken.literal); }
    std::unique_ptr<Ast::Expression> Parser::parseIntegerLiteral() { auto l = std::make_unique<Ast::IntegerLiteral>(); l->token = m_currentToken; try { l->value = std::stoll(m_currentToken.literal); } catch(...) { return nullptr; } return l; }
    std::unique_ptr<Ast::Expression> Parser::parseFloatLiteral() { auto l = std::make_unique<Ast::FloatLiteral>(); l->token = m_currentToken; try { l->value = std::stod(m_currentToken.literal); } catch(...) { return nullptr; } return l; }
    std::unique_ptr<Ast::Expression> Parser::parseBooleanLiteral() { auto l = std::make_unique<Ast::BooleanLiteral>(); l->token = m_currentToken; l->value = currentTokenIs(TokenType::True); return l; }
    std::unique_ptr<Ast::Expression> Parser::parseStringLiteral() { auto l = std::make_unique<Ast::StringLiteral>(); l->token = m_currentToken; l->value = m_currentToken.literal; return l; }

    std::unique_ptr<Ast::Expression> Parser::parseMacroReference() {
        auto macro = std::make_unique<Ast::MacroReference>();
        macro->token = m_currentToken;
        if (!expectPeek(TokenType::Identifier)) return nullptr;
        macro->name = parseIdentifier();
        return macro;
    }

    std::unique_ptr<Ast::Expression> Parser::parseArrayLiteral() {
        auto arr = std::make_unique<Ast::ArrayLiteral>();
        arr->token = m_currentToken;
        arr->elements = parseExpressionList(TokenType::RBracket);
        return arr;
    }

    std::unique_ptr<Ast::Expression> Parser::parseMapLiteral() {
        auto map = std::make_unique<Ast::MapLiteral>();
        map->token = m_currentToken; // The first '{'
        nextToken(); // Consume first '{'
        nextToken(); // Consume second '{'

        if (!peekTokenIs(TokenType::RBrace)) {
            do {
                auto kvp = parseKeyValueLiteral();
                if (kvp) map->elements.push_back(std::move(kvp));
                else return nullptr; // Error during kvp parsing
            } while (peekTokenIs(TokenType::Comma) && (nextToken(), true));
        }

        if (!expectPeek(TokenType::RBrace)) return nullptr;
        if (!expectPeek(TokenType::RBrace)) return nullptr;
        return map;
    }

    std::unique_ptr<Ast::KeyValueLiteral> Parser::parseKeyValueLiteral() {
        if (!expectPeek(TokenType::LBrace)) return nullptr;

        auto kvp = std::make_unique<Ast::KeyValueLiteral>();
        kvp->token = m_currentToken;

        if (!expectPeek(TokenType::Identifier)) return nullptr;
        kvp->key = parseIdentifier();

        if (!expectPeek(TokenType::Colon)) return nullptr;

        nextToken(); // Consume ':'
        kvp->value = parseExpression(LOWEST);

        if (!expectPeek(TokenType::RBrace)) return nullptr;

        return kvp;
    }

    std::unique_ptr<Ast::Expression> Parser::parseGroupedExpression() {
        nextToken();
        auto exp = parseExpression(LOWEST);
        if (!expectPeek(TokenType::RParen)) return nullptr;
        return exp;
    }

    std::unique_ptr<Ast::Expression> Parser::parseFunctionCall(std::unique_ptr<Ast::Expression> function) {
        auto call = std::make_unique<Ast::FunctionCall>();
        call->token = m_currentToken;
        // The function name must be an identifier
        if (auto* ident = dynamic_cast<Ast::Identifier*>(function.get())) {
             call->functionName = std::make_unique<Ast::Identifier>(ident->token, ident->value);
        } else {
            return nullptr; // Can't call a non-identifier
        }
        call->arguments = parseExpressionList(TokenType::RParen);
        return call;
    }

    std::vector<std::unique_ptr<Ast::Expression>> Parser::parseExpressionList(TokenType endToken) {
        std::vector<std::unique_ptr<Ast::Expression>> list;
        if (peekTokenIs(endToken)) {
            nextToken();
            return list;
        }
        nextToken();
        list.push_back(parseExpression(LOWEST));
        while (peekTokenIs(TokenType::Comma)) {
            nextToken();
            nextToken();
            list.push_back(parseExpression(LOWEST));
        }
        if (!expectPeek(endToken)) return {};
        return list;
    }
}
