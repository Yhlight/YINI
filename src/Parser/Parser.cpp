#include "YINI/Parser.hpp"
#include <stdexcept>

namespace YINI
{
    Parser::Parser(const std::string& content) : m_lexer(content)
    {
        nextToken();
    }

    void Parser::nextToken()
    {
        m_currentToken = m_lexer.getNextToken();
    }

    YiniDocument Parser::parse()
    {
        YiniDocument doc;
        while (m_currentToken.type != TokenType::Eof)
        {
            if (m_currentToken.type == TokenType::LeftBracket)
            {
                YiniSection section;
                parseSection(section);
                doc.addSection(std::move(section));
            }
            else
            {
                nextToken();
            }
        }
        return doc;
    }

    void Parser::parseSection(YiniSection& section)
    {
        nextToken(); // Consume '['
        if (m_currentToken.type == TokenType::Identifier)
        {
            section.name = m_currentToken.value;
            nextToken();
        }

        if (m_currentToken.type == TokenType::RightBracket)
        {
            nextToken(); // Consume ']'
        }

        if (m_currentToken.type == TokenType::Colon)
        {
            nextToken(); // Consume ':'
            while (m_currentToken.type == TokenType::Identifier)
            {
                section.inheritedSections.push_back(m_currentToken.value);
                nextToken();
                if (m_currentToken.type == TokenType::Comma)
                {
                    nextToken();
                }
                else
                {
                    break;
                }
            }
        }

        while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::Eof)
        {
            if (m_currentToken.type == TokenType::Identifier)
            {
                parseKeyValuePair(section);
            }
            else if (m_currentToken.type == TokenType::PlusEquals)
            {
                parseQuickRegistration(section);
            }
            else
            {
                nextToken();
            }
        }
    }

    void Parser::parseKeyValuePair(YiniSection& section)
    {
        YiniKeyValuePair pair;
        pair.key = m_currentToken.value;
        nextToken();

        if (m_currentToken.type == TokenType::Equals)
        {
            nextToken();
            pair.value = parseValue();
            section.pairs.push_back(std::move(pair));
        }
    }

    void Parser::parseQuickRegistration(YiniSection& section)
    {
        nextToken(); // Consume '+='
        section.registrationList.push_back(parseValue());
    }

    YiniValue Parser::parseValue()
    {
        YiniValue val;
        switch (m_currentToken.type)
        {
            case TokenType::String:
                val.data = m_currentToken.value;
                nextToken();
                break;
            case TokenType::Number:
                if (m_currentToken.value.find('.') != std::string::npos)
                {
                    val.data = std::stod(m_currentToken.value);
                }
                else
                {
                    val.data = std::stoi(m_currentToken.value);
                }
                nextToken();
                break;
            case TokenType::Boolean:
                val.data = (m_currentToken.value == "true");
                nextToken();
                break;
            case TokenType::LeftBracket:
                val.data = parseArray();
                break;
            default:
                nextToken();
                break;
        }
        return val;
    }

    std::unique_ptr<YiniArray> Parser::parseArray()
    {
        auto arr = std::make_unique<YiniArray>();
        nextToken(); // Consume '['

        while (m_currentToken.type != TokenType::RightBracket && m_currentToken.type != TokenType::Eof)
        {
            arr->elements.push_back(parseValue());

            if (m_currentToken.type == TokenType::Comma)
            {
                nextToken();
            }
        }

        if (m_currentToken.type == TokenType::RightBracket)
        {
            nextToken();
        }
        return arr;
    }
}