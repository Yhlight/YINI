#include "YINI/Parser.h"
#include "YINI/Ast.h"
#include <string>
#include <map>
#include <vector>

namespace YINI
{
    // Map of token types to their precedence levels
    static std::map<TokenType, Precedence> precedences = {
        {TokenType::Assign,    Precedence::EQUALS},
        {TokenType::Plus,      Precedence::SUM},
        {TokenType::Minus,     Precedence::SUM},
        {TokenType::Slash,     Precedence::PRODUCT},
        {TokenType::Asterisk,  Precedence::PRODUCT},
        {TokenType::Percent,   Precedence::PRODUCT},
        {TokenType::LeftParen, Precedence::CALL},
    };

    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        // Initialize with the first two tokens
        nextToken();
        nextToken();

        // Register prefix parsing functions
        registerPrefix(TokenType::Identifier, &Parser::parseIdentifier);
        registerPrefix(TokenType::Integer, &Parser::parseIntegerLiteral);
        registerPrefix(TokenType::Float, &Parser::parseFloatLiteral);
        registerPrefix(TokenType::Boolean, &Parser::parseBooleanLiteral);
        registerPrefix(TokenType::String, &Parser::parseStringLiteral);
        registerPrefix(TokenType::LeftBracket, &Parser::parseArrayLiteral);
        registerPrefix(TokenType::At, &Parser::parseMacroReference);
        registerPrefix(TokenType::LeftBrace, &Parser::parseMapLiteral);
        registerPrefix(TokenType::LeftParen, &Parser::parseGroupedOrCollectionExpression);

        // Register infix parsing functions
        registerInfix(TokenType::Plus, &Parser::parseInfixExpression);
        registerInfix(TokenType::Minus, &Parser::parseInfixExpression);
        registerInfix(TokenType::Slash, &Parser::parseInfixExpression);
        registerInfix(TokenType::Asterisk, &Parser::parseInfixExpression);
        registerInfix(TokenType::Percent, &Parser::parseInfixExpression);
        registerInfix(TokenType::LeftParen, &Parser::parseCallExpression);
    }

    void Parser::nextToken()
    {
        m_currentToken = m_peekToken;
        m_peekToken = m_lexer.NextToken();
    }

    void Parser::registerPrefix(TokenType type, prefixParseFn fn) {
        m_prefixParseFns[type] = fn;
    }

    void Parser::registerInfix(TokenType type, infixParseFn fn) {
        m_infixParseFns[type] = fn;
    }

    Precedence Parser::peekPrecedence()
    {
        if (precedences.count(m_peekToken.type)) {
            return precedences[m_peekToken.type];
        }
        return Precedence::LOWEST;
    }

    Precedence Parser::curPrecedence()
    {
        if (precedences.count(m_currentToken.type)) {
            return precedences[m_currentToken.type];
        }
        return Precedence::LOWEST;
    }

    std::vector<std::shared_ptr<Expression>> Parser::parseExpressionList(TokenType end)
    {
        std::vector<std::shared_ptr<Expression>> list;

        if (peekTokenIs(end)) {
            nextToken();
            return list;
        }

        nextToken();
        list.push_back(parseExpression(Precedence::LOWEST));

        while (peekTokenIs(TokenType::Comma)) {
            nextToken();
            nextToken();
            list.push_back(parseExpression(Precedence::LOWEST));
        }

        if (!expectPeek(end)) {
            return {};
        }

        return list;
    }

    std::unique_ptr<Program> Parser::ParseProgram()
    {
        auto program = std::make_unique<Program>();

        while (!currentTokenIs(TokenType::Eof))
        {
            auto stmt = parseStatement();
            if (stmt) {
                program->statements.push_back(stmt);
            } else {
                nextToken();
            }
        }
        return program;
    }

    std::shared_ptr<Statement> Parser::parseStatement()
    {
        if (currentTokenIs(TokenType::LeftBracket)) {
            if (peekTokenIs(TokenType::Identifier) && m_peekToken.literal == "#define") {
                return parseDefineStatement();
            }
            return parseSectionStatement();
        }
        return nullptr;
    }

    std::shared_ptr<Section> Parser::parseSectionStatement()
    {
        nextToken(); // consume '['
        if (!currentTokenIs(TokenType::Identifier)) return nullptr;

        auto section = std::make_shared<Section>();
        section->name = m_currentToken.literal;
        nextToken(); // consume name

        if (currentTokenIs(TokenType::Colon)) {
            nextToken(); // consume ':'

            while(true) {
                if (!currentTokenIs(TokenType::Identifier)) return nullptr;
                section->parents.push_back(std::dynamic_pointer_cast<Identifier>(parseIdentifier()));
                nextToken();

                if (currentTokenIs(TokenType::RightBracket)) break;
                if (!currentTokenIs(TokenType::Comma)) return nullptr;
                nextToken();
            }
        }

        if (!currentTokenIs(TokenType::RightBracket)) return nullptr;

        while (!peekTokenIs(TokenType::Eof) && !peekTokenIs(TokenType::LeftBracket)) {
            nextToken();
            if (currentTokenIs(TokenType::Identifier)) {
                auto pair = parseKeyValuePair();
                if (pair) section->pairs.push_back(pair);
            }
        }

        if (peekTokenIs(TokenType::LeftBracket)) {
            nextToken();
        }

        return section;
    }

    std::shared_ptr<DefineStatement> Parser::parseDefineStatement()
    {
        nextToken(); // consume '['
        nextToken(); // consume '#define'

        if (!currentTokenIs(TokenType::RightBracket)) return nullptr;

        auto stmt = std::make_shared<DefineStatement>();

        while (!peekTokenIs(TokenType::Eof) && !peekTokenIs(TokenType::LeftBracket)) {
            nextToken();
            if (currentTokenIs(TokenType::Identifier)) {
                auto pair = parseKeyValuePair();
                if (pair) stmt->pairs.push_back(pair);
            }
        }

        if (peekTokenIs(TokenType::LeftBracket)) {
            nextToken();
        }

        return stmt;
    }


    std::shared_ptr<KeyValuePair> Parser::parseKeyValuePair()
    {
        auto keyIdent = std::dynamic_pointer_cast<Identifier>(parseIdentifier());

        if (!expectPeek(TokenType::Assign)) {
            return nullptr;
        }

        nextToken();

        auto valueExpr = parseExpression(Precedence::LOWEST);

        auto pair = std::make_shared<KeyValuePair>();
        pair->key = keyIdent;
        pair->value = valueExpr;

        return pair;
    }

    std::shared_ptr<Expression> Parser::parseExpression(Precedence precedence)
    {
        if (m_prefixParseFns.find(m_currentToken.type) == m_prefixParseFns.end()) {
            return nullptr;
        }
        auto prefix = m_prefixParseFns.at(m_currentToken.type);

        auto leftExp = (this->*prefix)();

        while (!peekTokenIs(TokenType::Eof) && precedence < peekPrecedence()) {
            if (m_infixParseFns.find(m_peekToken.type) == m_infixParseFns.end()) {
                return leftExp;
            }
            auto infix = m_infixParseFns.at(m_peekToken.type);

            nextToken();

            leftExp = (this->*infix)(leftExp);
        }

        return leftExp;
    }

    std::shared_ptr<Expression> Parser::parseInfixExpression(std::shared_ptr<Expression> left)
    {
        auto expression = std::make_shared<InfixExpression>();
        expression->left = left;
        expression->op = m_currentToken.literal;

        auto precedence = curPrecedence();
        nextToken();
        expression->right = parseExpression(precedence);

        return expression;
    }

    std::shared_ptr<Expression> Parser::parseCallExpression(std::shared_ptr<Expression> function)
    {
        auto call = std::make_shared<CallExpression>();
        call->function = function;
        call->arguments = parseExpressionList(TokenType::RightParen);
        return call;
    }

    std::shared_ptr<Expression> Parser::parseGroupedOrCollectionExpression()
    {
        nextToken(); // Consume '('

        // Handle empty collection `()`
        if (currentTokenIs(TokenType::RightParen)) {
            return std::make_shared<CollectionLiteral>();
        }

        auto first_expr = parseExpression(Precedence::LOWEST);

        // If the next token is not a comma, it must be a grouped expression.
        if (!peekTokenIs(TokenType::Comma)) {
            if (!expectPeek(TokenType::RightParen)) return nullptr;
            return first_expr;
        }

        // If we get here, it's a collection because we saw a comma.
        auto collection = std::make_shared<CollectionLiteral>();
        collection->elements.push_back(first_expr);

        while (peekTokenIs(TokenType::Comma)) {
            nextToken(); // consume previous expr token
            nextToken(); // consume comma

            // Handle trailing comma
            if (currentTokenIs(TokenType::RightParen)) {
                break;
            }

            collection->elements.push_back(parseExpression(Precedence::LOWEST));
        }

        if (!currentTokenIs(TokenType::RightParen)) {
             if (!expectPeek(TokenType::RightParen)) return nullptr;
        }

        return collection;
    }

    std::shared_ptr<Expression> Parser::parseIdentifier()
    {
        auto ident = std::make_shared<Identifier>();
        ident->value = m_currentToken.literal;
        return ident;
    }

    std::shared_ptr<Expression> Parser::parseIntegerLiteral()
    {
        auto literal = std::make_shared<IntegerLiteral>();
        try {
            literal->value = std::stoll(m_currentToken.literal);
        } catch (...) {
            return nullptr;
        }
        return literal;
    }

    std::shared_ptr<Expression> Parser::parseFloatLiteral()
    {
        auto literal = std::make_shared<FloatLiteral>();
        try {
            literal->value = std::stod(m_currentToken.literal);
        } catch (...) {
            return nullptr;
        }
        return literal;
    }

    std::shared_ptr<Expression> Parser::parseBooleanLiteral()
    {
        auto literal = std::make_shared<BooleanLiteral>();
        literal->value = (m_currentToken.literal == "true");
        return literal;
    }

    std::shared_ptr<Expression> Parser::parseStringLiteral()
    {
        auto literal = std::make_shared<StringLiteral>();
        literal->value = m_currentToken.literal;
        return literal;
    }

    std::shared_ptr<Expression> Parser::parseArrayLiteral()
    {
        auto array = std::make_shared<ArrayLiteral>();
        array->elements = parseExpressionList(TokenType::RightBracket);
        return array;
    }

    std::shared_ptr<Expression> Parser::parseMacroReference()
    {
        nextToken(); // consume '@'
        if (!currentTokenIs(TokenType::Identifier)) return nullptr;

        auto macro = std::make_shared<MacroReference>();
        macro->name = m_currentToken.literal;
        return macro;
    }

    std::shared_ptr<Expression> Parser::parseMapLiteral()
    {
        auto map = std::make_shared<MapLiteral>();

        while (!peekTokenIs(TokenType::RightBrace) && !peekTokenIs(TokenType::Eof)) {
            nextToken(); // consume '{' or ','

            auto key = parseExpression(Precedence::LOWEST);
            if (!key) return nullptr;

            if (!expectPeek(TokenType::Colon)) return nullptr;

            nextToken(); // consume ':'

            auto value = parseExpression(Precedence::LOWEST);
            if (!value) return nullptr;

            map->pairs[key] = value;

            if (!peekTokenIs(TokenType::RightBrace) && !expectPeek(TokenType::Comma)) {
                return nullptr;
            }
        }

        if (!expectPeek(TokenType::RightBrace)) return nullptr;

        return map;
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
            return false;
        }
    }
}