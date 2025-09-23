#include "Lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string& source)
    : source_code_(source)
{
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (!isAtEnd())
    {
        start_ = current_;
        skipWhitespaceAndComments();
        if (isAtEnd())
        {
            break;
        }

        start_ = current_;
        char c = advance();

        switch (c)
        {
            case '[': tokens.push_back(makeToken(TokenType::L_BRACKET)); break;
            case ']': tokens.push_back(makeToken(TokenType::R_BRACKET)); break;
            case '(': tokens.push_back(makeToken(TokenType::L_PAREN)); break;
            case ')': tokens.push_back(makeToken(TokenType::R_PAREN)); break;
            case '{': tokens.push_back(makeToken(TokenType::L_BRACE)); break;
            case '}': tokens.push_back(makeToken(TokenType::R_BRACE)); break;
            case ',': tokens.push_back(makeToken(TokenType::COMMA)); break;
            case ':': tokens.push_back(makeToken(TokenType::COLON)); break;
            case '=': tokens.push_back(makeToken(TokenType::EQUAL)); break;
            case '@': tokens.push_back(makeToken(TokenType::AT)); break;
            case '#':
                tokens.push_back(makeToken(TokenType::HASH));
                tokens.push_back(scanHexLiteral());
                break;
            case '+':
                if (peek() == '=')
                {
                    advance();
                    tokens.push_back(makeToken(TokenType::PLUS_EQUAL));
                }
                else
                {
                    tokens.push_back(makeToken(TokenType::UNEXPECTED, "+"));
                }
                break;
            case '"':
                tokens.push_back(scanString());
                break;
            default:
                if (std::isalpha(c) || c == '_')
                {
                    tokens.push_back(scanIdentifier());
                }
                else if (std::isdigit(c))
                {
                    tokens.push_back(scanNumber());
                }
                else
                {
                    tokens.push_back(makeToken(TokenType::UNEXPECTED, std::string(1, c)));
                }
        }
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line_, column_});
    return tokens;
}

char Lexer::peek()
{
    if (isAtEnd())
    {
        return '\0';
    }
    return source_code_[current_];
}

char Lexer::advance()
{
    column_++;
    return source_code_[current_++];
}

bool Lexer::isAtEnd()
{
    return current_ >= source_code_.length();
}

void Lexer::skipWhitespaceAndComments()
{
    while (!isAtEnd())
    {
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                line_++;
                column_ = 1;
                break;
            case '/':
                if (current_ + 1 < source_code_.length() && source_code_[current_ + 1] == '/')
                {
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else if (current_ + 1 < source_code_.length() && source_code_[current_ + 1] == '*')
                {
                    advance();
                    advance();
                    while (!isAtEnd())
                    {
                        if (peek() == '*' && current_ + 1 < source_code_.length() && source_code_[current_ + 1] == '/')
                        {
                            advance();
                            advance();
                            break;
                        }
                        if (peek() == '\n')
                        {
                            line_++;
                            column_ = 1;
                        }
                        advance();
                    }
                }
                else
                {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token Lexer::makeToken(TokenType type)
{
    return makeToken(type, source_code_.substr(start_, current_ - start_));
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme)
{
    return {type, lexeme, line_, (int)(column_ - lexeme.length())};
}

Token Lexer::scanIdentifier()
{
    while (std::isalnum(peek()) || peek() == '_')
    {
        advance();
    }
    std::string text = source_code_.substr(start_, current_ - start_);
    return makeToken(TokenType::IDENTIFIER, text);
}

Token Lexer::scanHexLiteral()
{
    start_ = current_;
    while (std::isxdigit(peek()))
    {
        advance();
    }
    return makeToken(TokenType::IDENTIFIER);
}

Token Lexer::scanString()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
        {
            line_++;
            column_ = 1;
        }
        advance();
    }

    if (isAtEnd())
    {
        return makeToken(TokenType::UNEXPECTED, "Unterminated string.");
    }

    advance();
    std::string value = source_code_.substr(start_ + 1, current_ - start_ - 2);
    return makeToken(TokenType::STRING, value);
}

Token Lexer::scanNumber()
{
    bool is_float = false;
    while (std::isdigit(peek()))
    {
        advance();
    }

    if (peek() == '.' && std::isdigit(source_code_[current_ + 1]))
    {
        is_float = true;
        advance();
        while (std::isdigit(peek()))
        {
            advance();
        }
    }

    std::string number_str = source_code_.substr(start_, current_ - start_);
    return makeToken(is_float ? TokenType::FLOAT : TokenType::INTEGER, number_str);
}
