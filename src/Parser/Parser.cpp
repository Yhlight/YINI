#include "Parser.h"
#include <stdexcept>

namespace YINI
{
    Parser::Parser(Lexer& lexer) : m_lexer(lexer)
    {
        Token token;
        do
        {
            token = m_lexer.getNextToken();
            m_tokens.push_back(token);
        } while (token.type != TokenType::EndOfFile);

        m_current_token = m_tokens[0];
    }

    void Parser::advance()
    {
        if (m_token_pos < m_tokens.size() - 1)
        {
            m_token_pos++;
            m_current_token = m_tokens[m_token_pos];
        }
    }

    bool Parser::match(TokenType type)
    {
        if (peek().type == type)
        {
            advance();
            return true;
        }
        return false;
    }

    void Parser::consume(TokenType type, const std::string& message)
    {
        if (peek().type == type)
        {
            advance();
        }
        else
        {
            throw std::runtime_error(message);
        }
    }

    Token Parser::peek() const
    {
        return m_current_token;
    }

    std::unique_ptr<YiniFile> Parser::parse()
    {
        auto yini_file = std::make_unique<YiniFile>();
        while (peek().type != TokenType::EndOfFile)
        {
            if (peek().type == TokenType::Section || peek().type == TokenType::Define || peek().type == TokenType::Include)
            {
                auto section = parseSection();
                if (section)
                {
                    yini_file->sections[section->name] = std::move(*section);
                }
            }
            else
            {
                // Or handle top-level key-value pairs if the language supports it.
                // For now, we'll just skip tokens that are not part of a section.
                advance();
            }
        }
        return yini_file;
    }

    std::unique_ptr<Section> Parser::parseSection()
    {
        auto section = std::make_unique<Section>();

        // At this point, we have already seen a Section, Define, or Include token.
        Token section_token = peek();
        section->name = section_token.text;

        if (section_token.type == TokenType::Define)
        {
            section->is_define_section = true;
        }
        else if (section_token.type == TokenType::Include)
        {
            section->is_include_section = true;
        }

        advance(); // Consume the section name token

        // Parse inheritance
        if (match(TokenType::Colon))
        {
            do
            {
                if (peek().type == TokenType::Identifier)
                {
                    section->inherits.push_back(peek().text);
                    advance();
                }
                else
                {
                    throw std::runtime_error("Expected identifier after ':' or ',' in section inheritance.");
                }
            } while (match(TokenType::Comma));
        }

        // Parse key-value pairs
        while (peek().type != TokenType::EndOfFile &&
               peek().type != TokenType::Section &&
               peek().type != TokenType::Define &&
               peek().type != TokenType::Include)
        {
            auto kvp = parseKeyValuePair();
            if (kvp)
            {
                section->pairs.push_back(std::move(*kvp));
            }
        }

        return section;
    }

    std::unique_ptr<KeyValuePair> Parser::parseKeyValuePair()
    {
        auto kvp = std::make_unique<KeyValuePair>();

        if (peek().type == TokenType::PlusAssign) // Quick registration
        {
            kvp->is_quick_registration = true;
            advance(); // consume '+='
            kvp->key = ""; // No key for quick registration
            kvp->value = parseValue();
        }
        else if (peek().type == TokenType::Identifier)
        {
            kvp->is_quick_registration = false;
            kvp->key = peek().text;
            advance(); // consume identifier

            consume(TokenType::Assign, "Expected '=' after key.");

            kvp->value = parseValue();
        }
        else
        {
            // Not a valid start for a key-value pair, maybe an empty line.
            advance();
            return nullptr;
        }

        return kvp;
    }

    std::unique_ptr<Value> Parser::parseValue()
    {
        auto value = std::make_unique<Value>();
        switch (peek().type)
        {
            case TokenType::String:
                value->data = String(peek().text);
                advance();
                break;
            case TokenType::Integer:
                value->data = Integer(std::stoll(peek().text));
                advance();
                break;
            case TokenType::Float:
                value->data = Float(std::stod(peek().text));
                advance();
                break;
            case TokenType::Boolean:
                value->data = Boolean(peek().text == "true");
                advance();
                break;
            case TokenType::LeftBracket:
                return parseArray();
            case TokenType::LeftBrace:
                return parseMap();
            case TokenType::LeftParen:
                return parseCoordinate();
            case TokenType::Color:
            {
                // Assuming #RRGGBB format
                long long color_val = std::stoll(peek().text, nullptr, 16);
                value->data = Color{(int)((color_val >> 16) & 0xFF), (int)((color_val >> 8) & 0xFF), (int)(color_val & 0xFF)};
                advance();
                break;
            }
            case TokenType::Macro:
                value->data = Macro{peek().text};
                advance();
                break;
            case TokenType::Identifier:
            {
                if (peek().text == "color" || peek().text == "Color")
                {
                    // This is a color function
                    advance(); // consume identifier
                    consume(TokenType::LeftParen, "Expected '(' after color function.");
                    int r = std::stoi(peek().text);
                    consume(TokenType::Integer, "Expected integer for red component.");
                    consume(TokenType::Comma, "Expected ',' after red component.");
                    int g = std::stoi(peek().text);
                    consume(TokenType::Integer, "Expected integer for green component.");
                    consume(TokenType::Comma, "Expected ',' after green component.");
                    int b = std::stoi(peek().text);
                    consume(TokenType::Integer, "Expected integer for blue component.");
                    consume(TokenType::RightParen, "Expected ')' after blue component.");
                    value->data = Color{r, g, b};
                }
                else
                {
                    // It could be a reference to another key, but YINI spec doesn't mention this.
                    // For now, we treat it as an error or an implicit string.
                    // Let's treat it as a string for now.
                    value->data = String(peek().text);
                    advance();
                }
                break;
            }
            default:
                throw std::runtime_error("Unexpected token when parsing value: " + peek().text);
        }
        return value;
    }

    std::unique_ptr<Value> Parser::parseArray()
    {
        auto value = std::make_unique<Value>();
        Array array;
        consume(TokenType::LeftBracket, "Expected '[' to start an array.");

        if (peek().type != TokenType::RightBracket)
        {
            do
            {
                array.push_back(parseValue());
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightBracket, "Expected ']' to end an array.");
        value->data = std::move(array);
        return value;
    }

    std::unique_ptr<Value> Parser::parseMap()
    {
        auto value = std::make_unique<Value>();
        Map map;
        consume(TokenType::LeftBrace, "Expected '{' to start a map.");

        if (peek().type != TokenType::RightBrace)
        {
            do
            {
                std::string key = peek().text;
                consume(TokenType::String, "Expected string literal for map key."); // Assuming keys are strings
                consume(TokenType::Colon, "Expected ':' after map key.");
                map[key] = parseValue();
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightBrace, "Expected '}' to end a map.");
        value->data = std::move(map);
        return value;
    }

    std::unique_ptr<Value> Parser::parseCoordinate()
    {
        auto value = std::make_unique<Value>();
        Coordinate coord;
        consume(TokenType::LeftParen, "Expected '(' to start a coordinate.");

        coord.x = std::stod(peek().text);
        // Should check for Integer or Float token
        advance();
        consume(TokenType::Comma, "Expected ',' after x coordinate.");

        coord.y = std::stod(peek().text);
        advance();

        if (match(TokenType::Comma))
        {
            coord.z = std::stod(peek().text);
            advance();
            coord.has_z = true;
        }
        else
        {
            coord.z = 0;
            coord.has_z = false;
        }

        consume(TokenType::RightParen, "Expected ')' to end a coordinate.");
        value->data = coord;
        return value;
    }
}
