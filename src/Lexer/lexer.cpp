#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& input) : input(input), position(0), line(1), column_start(1) {}

void Lexer::skipWhitespace() {
    while (position < input.length() && isspace(input[position])) {
        if (input[position] == '\n') {
            line++;
            column_start = 1;
        } else {
            column_start++;
        }
        position++;
    }
}

Token Lexer::nextToken() {
    skipWhitespace();

    size_t start_col = column_start;

    if (position >= input.length()) {
        return {TokenType::EndOfFile, "", line, start_col};
    }

    // Handle comments before other tokens
    if (input[position] == '/') {
        if (position + 1 < input.length()) {
            if (input[position + 1] == '/') {
                // Single-line comment
                while (position < input.length() && input[position] != '\n') {
                    position++;
                    column_start++;
                }
                return nextToken(); // Get the next token after the comment
            } else if (input[position + 1] == '*') {
                // Multi-line comment
                position += 2; // Skip "/*"
                column_start += 2;
                while (position + 1 < input.length() &&
                       !(input[position] == '*' && input[position + 1] == '/')) {
                    if (input[position] == '\n') {
                        line++;
                        column_start = 1;
                    } else {
                        column_start++;
                    }
                    position++;
                }
                if (position + 1 < input.length()) {
                    position += 2; // Skip "*/"
                    column_start += 2;
                } else {
                    return {TokenType::Unexpected, "Unterminated multi-line comment", line, start_col};
                }
                return nextToken(); // Get the next token after the comment
            }
        }
    }

    if (input[position] == '+' && position + 1 < input.length() && input[position + 1] == '=') {
        position += 2;
        column_start += 2;
        return {TokenType::PlusEquals, "+=", line, start_col};
    }

    char currentChar = input[position];

    // Identifier, true, false
    if (isalpha(currentChar) || currentChar == '_' || currentChar == '#') {
        size_t start = position;
        while (position < input.length() && (isalnum(input[position]) || input[position] == '_' || input[position] == '#')) {
            position++;
            column_start++;
        }
        std::string value = input.substr(start, position - start);
        if (value == "true" || value == "false") {
            return {TokenType::Boolean, value, line, start_col};
        }
        return {TokenType::Identifier, value, line, start_col};
    }

    // Equals: =
    if (currentChar == '=') {
        position++;
        column_start++;
        return {TokenType::Equals, "=", line, start_col};
    }

    // String: "value"
    if (currentChar == '"') {
        size_t start = ++position;
        column_start++;
        while (position < input.length() && input[position] != '"') {
            position++;
            column_start++;
        }
        if (position < input.length()) {
            std::string value = input.substr(start, position - start);
            position++; // Skip closing '"'
            column_start++;
            return {TokenType::String, value, line, start_col};
        }
    }

    // Number: 123, 3.14
    if (isdigit(currentChar) || (currentChar == '.' && position + 1 < input.length() && isdigit(input[position+1]))) {
        size_t start = position;
        while (position < input.length() && (isdigit(input[position]) || input[position] == '.')) {
            position++;
            column_start++;
        }
        return {TokenType::Number, input.substr(start, position - start), line, start_col};
    }

    // Punctuation
    auto create_token = [&](TokenType type, const std::string& value) {
        position++;
        column_start++;
        return Token{type, value, line, start_col};
    };

    switch (currentChar) {
        case '[': return create_token(TokenType::LeftBracket, "[");
        case ']': return create_token(TokenType::RightBracket, "]");
        case '(': return create_token(TokenType::LeftParen, "(");
        case ')': return create_token(TokenType::RightParen, ")");
        case '+': return create_token(TokenType::Plus, "+");
        case '-': return create_token(TokenType::Minus, "-");
        case '*': return create_token(TokenType::Star, "*");
        case '/': return create_token(TokenType::Slash, "/");
        case '%': return create_token(TokenType::Percent, "%");
        case '{': return create_token(TokenType::LeftBrace, "{");
        case '}': return create_token(TokenType::RightBrace, "}");
        case ',': return create_token(TokenType::Comma, ",");
        case ':': return create_token(TokenType::Colon, ":");
        case '@': return create_token(TokenType::At, "@");
        case '$': return create_token(TokenType::Dollar, "$");
        case '#': return create_token(TokenType::Hash, "#");
        case '.': return create_token(TokenType::Dot, ".");
        case '!': return create_token(TokenType::Bang, "!");
        case '?': return create_token(TokenType::Question, "?");
    }

    // If no token is matched, return unexpected
    position++;
    column_start++;
    return {TokenType::Unexpected, std::string(1, currentChar), line, start_col};
}

std::vector<Token> Lexer::allTokens() {
    std::vector<Token> tokens;
    Token token = nextToken();
    while (token.type != TokenType::EndOfFile) {
        tokens.push_back(token);
        token = nextToken();
    }
    return tokens;
}