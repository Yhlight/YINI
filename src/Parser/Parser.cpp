#include "Parser.h"
#include "YiniValue.h"
#include <memory>
#include <stdexcept>
#include <iostream>

namespace Yini
{
    namespace
    {
        float get_numeric_value(const YiniValue& val)
        {
            const auto& variant = val.getVariant();
            if (std::holds_alternative<YiniInteger>(variant))
            {
                return static_cast<float>(std::get<YiniInteger>(variant));
            }
            else if (std::holds_alternative<YiniFloat>(variant))
            {
                return static_cast<float>(std::get<YiniFloat>(variant));
            }
            return 0.0f;
        }

        int hex_char_to_int(char c)
        {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }
    }


    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        nextToken();
        nextToken();
    }

    void Parser::nextToken()
    {
        m_curToken = m_peekToken;
        m_peekToken = m_lexer.nextToken();
    }

    YiniData Parser::parseYini()
    {
        YiniData data;
        YiniSection* currentSection = nullptr;

        while (m_curToken.type != TokenType::EndOfFile)
        {
            if (m_curToken.type == TokenType::LBracket)
            {
                currentSection = parseSection(data);
            }
            else if (currentSection)
            {
                if (currentSection->getName() == "#include")
                {
                    if (m_curToken.type == TokenType::PlusAssign)
                    {
                        nextToken(); // consume +=
                        if(m_curToken.type == TokenType::String)
                        {
                            data.addInclude(m_curToken.literal);
                            nextToken();
                        }
                        else
                        {
                             m_errors.push_back("Expected string literal for include path");
                        }
                    }
                    else
                    {
                        nextToken();
                    }
                }
                else if (m_curToken.type == TokenType::Identifier && m_peekToken.type == TokenType::Assign)
                {
                    parseKeyValuePair(*currentSection, data);
                }
                else if (m_curToken.type == TokenType::PlusAssign)
                {
                    parsePlusEqual(*currentSection, data);
                }
                else
                {
                    nextToken();
                }
            }
            else
            {
                nextToken();
            }
        }

        return data;
    }

    YiniSection* Parser::parseSection(YiniData& data)
    {
        nextToken();

        std::string sectionName;
        if(m_curToken.type == TokenType::Hash)
        {
            sectionName += "#";
            nextToken();
        }

        if(m_curToken.type != TokenType::Identifier)
        {
            m_errors.push_back("Expected identifier in section name");
            return nullptr;
        }
        sectionName += m_curToken.literal;
        nextToken();

        if(m_curToken.type != TokenType::RBracket)
        {
             m_errors.push_back("Expected ']' after section name");
            return nullptr;
        }

        if(!data.getSection(sectionName))
        {
            YiniSection section(sectionName);
            data.addSection(section);
        }
        YiniSection* newSection = data.getSection(sectionName);

        if (m_peekToken.type == TokenType::Colon)
        {
            nextToken();
            nextToken();

            while(m_curToken.type == TokenType::Identifier)
            {
                newSection->addInheritance(m_curToken.literal);
                nextToken();

                if (m_curToken.type == TokenType::Comma)
                {
                    nextToken();
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            nextToken();
        }

        return newSection;
    }

    void Parser::parseKeyValuePair(YiniSection& section, YiniData& data)
    {
        std::string key = m_curToken.literal;
        nextToken();
        nextToken();

        YiniValue value = parseValue(data);

        if(section.getName() == "#define")
        {
            data.addMacro(key, value);
        }
        else
        {
            section.addKeyValuePair(key, value);
        }
    }

    void Parser::parsePlusEqual(YiniSection& section, YiniData& data)
    {
        nextToken();
        YiniValue value = parseValue(data);
        section.addValue(value);
    }

    YiniValue Parser::parseValue(YiniData& data)
    {
        if (m_curToken.type == TokenType::At)
        {
            nextToken();
            if (m_curToken.type != TokenType::Identifier)
            {
                m_errors.push_back("Expected identifier after '@'");
                return {};
            }
            std::string macroName = m_curToken.literal;
            nextToken();

            auto macros = data.getMacros();
            auto it = macros.find(macroName);
            if(it != macros.end())
            {
                return it->second;
            }
            else
            {
                m_errors.push_back("Undefined macro: " + macroName);
                return {};
            }
        }

        YiniValue val;
        switch (m_curToken.type)
        {
            case TokenType::String:
                val.set<YiniString>(m_curToken.literal);
                break;
            case TokenType::Integer:
                val.set<YiniInteger>(std::stoll(m_curToken.literal));
                break;
            case TokenType::Float:
                val.set<YiniFloat>(std::stod(m_curToken.literal));
                break;
            case TokenType::Boolean:
                val.set<YiniBoolean>(m_curToken.literal == "true");
                break;
            case TokenType::Identifier:
                 if (m_curToken.literal == "Color" || m_curToken.literal == "color")
                {
                    return parseColor(data);
                }
                val.set<YiniString>(m_curToken.literal);
                break;
            case TokenType::LBracket:
                return parseArray(data);
            case TokenType::LParen:
                return parseCoordinate(data);
            case TokenType::LBrace:
                return parseMap(data);
            case TokenType::Hash:
                return parseColor(data);
            default:
                val.set<YiniString>(m_curToken.literal);
                break;
        }
        nextToken();
        return val;
    }

    YiniValue Parser::parseArray(YiniData& data)
    {
        YiniArray array;
        nextToken();

        while (m_curToken.type != TokenType::RBracket && m_curToken.type != TokenType::EndOfFile)
        {
            array.push_back(parseValue(data));
            if (m_curToken.type == TokenType::Comma)
            {
                nextToken();
            }
            else if (m_curToken.type != TokenType::RBracket)
            {
                m_errors.push_back("Expected ',' or ']' in array");
                break;
            }
        }

        if(m_curToken.type != TokenType::RBracket)
        {
            m_errors.push_back("Expected ']' at end of array");
        }
        nextToken();

        YiniValue val;
        val.set<YiniArray>(array);
        return val;
    }

    YiniValue Parser::parseCoordinate(YiniData& data)
    {
        nextToken();

        YiniValue x_val = parseValue(data);
        if (m_curToken.type != TokenType::Comma) { m_errors.push_back("Expected ',' after x coordinate"); return {}; }
        nextToken();
        YiniValue y_val = parseValue(data);

        float x = get_numeric_value(x_val);
        float y = get_numeric_value(y_val);

        if (m_curToken.type == TokenType::Comma)
        {
            nextToken();
            YiniValue z_val = parseValue(data);
            if (m_curToken.type != TokenType::RParen) { m_errors.push_back("Expected ')' after z coordinate"); return {}; }
            nextToken();
            float z = get_numeric_value(z_val);

            YiniValue val;
            val.set<Coordinate3D>({x, y, z});
            return val;
        }
        else if (m_curToken.type == TokenType::RParen)
        {
            nextToken();
            YiniValue val;
            val.set<Coordinate2D>({x, y});
            return val;
        }
        else
        {
            m_errors.push_back("Expected ')' or ',' after y coordinate");
            return {};
        }
    }

    YiniValue Parser::parseMap(YiniData& data)
    {
        YiniMap map;
        nextToken();

        while (m_curToken.type != TokenType::RBrace && m_curToken.type != TokenType::EndOfFile)
        {
            if (m_curToken.type != TokenType::Identifier)
            {
                m_errors.push_back("Expected identifier as map key");
                break;
            }
            std::string key = m_curToken.literal;
            nextToken();

            if (m_curToken.type != TokenType::Colon)
            {
                m_errors.push_back("Expected ':' after map key");
                break;
            }
            nextToken();

            map[key] = parseValue(data);

            if (m_curToken.type == TokenType::Comma)
            {
                nextToken();
            }
            else if (m_curToken.type != TokenType::RBrace)
            {
                m_errors.push_back("Expected ',' or '}' in map");
                break;
            }
        }

        if(m_curToken.type != TokenType::RBrace)
        {
            m_errors.push_back("Expected '}' at end of map");
        }
        nextToken();

        YiniValue val;
        val.set<YiniMap>(map);
        return val;
    }

    YiniValue Parser::parseColor(YiniData& data)
    {
        if (m_curToken.type == TokenType::Hash)
        {
            nextToken();
            if (m_curToken.type != TokenType::Identifier || m_curToken.literal.length() != 6)
            {
                m_errors.push_back("Invalid hex color format");
                return {};
            }
            std::string hex = m_curToken.literal;
            uint8_t r = hex_char_to_int(hex[0]) * 16 + hex_char_to_int(hex[1]);
            uint8_t g = hex_char_to_int(hex[2]) * 16 + hex_char_to_int(hex[3]);
            uint8_t b = hex_char_to_int(hex[4]) * 16 + hex_char_to_int(hex[5]);
            nextToken();

            YiniValue val;
            val.set<ColorRGB>({r, g, b});
            return val;
        }
        else if (m_curToken.type == TokenType::Identifier && (m_curToken.literal == "Color" || m_curToken.literal == "color"))
        {
            nextToken();
            if (m_curToken.type != TokenType::LParen) { m_errors.push_back("Expected '(' after color identifier"); return {}; }
            nextToken();

            auto r_val = parseValue(data);
            if (m_curToken.type != TokenType::Comma) { m_errors.push_back("Expected ',' after r value"); return {}; }
            nextToken();

            auto g_val = parseValue(data);
            if (m_curToken.type != TokenType::Comma) { m_errors.push_back("Expected ',' after g value"); return {}; }
            nextToken();

            auto b_val = parseValue(data);
            if (m_curToken.type != TokenType::RParen) { m_errors.push_back("Expected ')' after b value"); return {}; }
            nextToken();

            YiniValue val;
            val.set<ColorRGB>({
                (uint8_t)get_numeric_value(r_val),
                (uint8_t)get_numeric_value(g_val),
                (uint8_t)get_numeric_value(b_val)
            });
            return val;
        }

        m_errors.push_back("Unknown color format");
        return {};
    }


    const std::vector<std::string>& Parser::getErrors() const
    {
        return m_errors;
    }

    bool Parser::expectPeek(TokenType t)
    {
        if (m_peekToken.type == t)
        {
            nextToken();
            return true;
        }
        else
        {
            m_errors.push_back("Expected next token to be " + std::to_string((int)t) + ", got " + std::to_string((int)m_peekToken.type) + " instead");
            return false;
        }
    }
}
