#include "Lexer.h"
#include <cctype>
#include <map>

namespace Yini
{

static std::map<std::string, TokenType> keywords;

Lexer::Lexer(const std::string& source) : source(source) {
    keywords["true"] = TokenType::True;
    keywords["false"] = TokenType::False;
    keywords["List"] = TokenType::List;
    keywords["list"] = TokenType::List;
    keywords["Array"] = TokenType::Array;
    keywords["array"] = TokenType::Array;
    keywords["color"] = TokenType::Color;
    keywords["Color"] = TokenType::Color;
    keywords["coord"] = TokenType::Coord;
    keywords["Coord"] = TokenType::Coord;
    keywords["path"] = TokenType::Path;
    keywords["Path"] = TokenType::Path;
}

std::vector<Token> Lexer::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    tokens.push_back({TokenType::Eof, "", {}, line});
    return tokens;
}

bool Lexer::isAtEnd()
{
    return current >= source.length();
}

char Lexer::advance()
{
    current++;
    return source[current - 1];
}

void Lexer::addToken(TokenType type)
{
    addToken(type, {});
}

void Lexer::addToken(TokenType type, const std::variant<std::monostate, std::string, double, bool>& literal) {
    std::string text = source.substr(start, current - start);
    tokens.push_back({type, text, literal, line});
}


char Lexer::peek()
{
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}


bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        // Unterminated string.
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::String, value);
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isdigit(peekNext())) {
        // Consume the ".'
        advance();

        while (isdigit(peek())) advance();
    }

    addToken(TokenType::Number, std::stod(source.substr(start, current - start)));
}

void Lexer::scanToken()
{
    char c = advance();
    switch (c)
    {
        case '(': addToken(TokenType::LeftParen); break;
        case ')': addToken(TokenType::RightParen); break;
        case '{': addToken(TokenType::LeftBrace); break;
        case '}': addToken(TokenType::RightBrace); break;
        case '[': addToken(TokenType::LeftBracket); break;
        case ']': addToken(TokenType::RightBracket); break;
        case ':': addToken(TokenType::Colon); break;
        case ',': addToken(TokenType::Comma); break;
        case '=': addToken(TokenType::Equal); break;
        case '@': addToken(TokenType::At); break;
        case '+':
             if (match('=')) {
                 addToken(TokenType::PlusEqual);
             } else {
                 addToken(TokenType::Plus);
             }
             break;
        case '"': string(); break;
        case '#':
            // This is a more robust check for a hex color.
            // It must be followed by exactly 6 hex digits, and not be part of a larger identifier.
            if (current + 6 <= source.length()) {
                bool is_hex = true;
                for (int i = 0; i < 6; ++i) {
                    if (!isxdigit(source[current + i])) {
                        is_hex = false;
                        break;
                    }
                }
                if (is_hex && (current + 6 == source.length() || !isalnum(source[current + 6]))) {
                    current += 6;
                    addToken(TokenType::HexColor);
                    break;
                }
            }
            // Otherwise, it's just a hash for a special section.
            addToken(TokenType::Hash);
            break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;
        case '\n':
            line++;
            break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                while (isalnum(peek()) || peek() == '_') advance();

                std::string text = source.substr(start, current - start);
                auto it = keywords.find(text);
                if (it != keywords.end()) {
                    if (it->second == TokenType::True) {
                        addToken(TokenType::True, true);
                    } else if (it->second == TokenType::False) {
                        addToken(TokenType::False, false);
                    } else {
                         addToken(it->second);
                    }
                } else {
                    addToken(TokenType::Identifier);
                }
            }
            break;
    }
}

} // namespace Yini