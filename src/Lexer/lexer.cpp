#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& input) : input(input), position(0) {}

void Lexer::skipWhitespace() {
    while (position < input.length() && isspace(input[position])) {
        position++;
    }
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (position >= input.length()) {
        return {TokenType::EndOfFile, ""};
    }

    // Handle comments before other tokens
    if (input[position] == '/') {
        if (position + 1 < input.length()) {
            if (input[position + 1] == '/') {
                // Single-line comment
                while (position < input.length() && input[position] != '\n') {
                    position++;
                }
                return nextToken(); // Get the next token after the comment
            } else if (input[position + 1] == '*') {
                // Multi-line comment
                position += 2; // Skip "/*"
                while (position + 1 < input.length() &&
                       !(input[position] == '*' && input[position + 1] == '/')) {
                    position++;
                }
                if (position + 1 < input.length()) {
                    position += 2; // Skip "*/"
                } else {
                    // Unterminated multi-line comment
                    return {TokenType::Unexpected, "Unterminated multi-line comment"};
                }
                return nextToken(); // Get the next token after the comment
            }
        }
    }

    char currentChar = input[position];

    // Section: [Header]
    if (currentChar == '[') {
        size_t start = ++position;
        while (position < input.length() && input[position] != ']') {
            position++;
        }
        if (position < input.length()) {
            std::string value = input.substr(start, position - start);
            position++; // Skip ']'
            return {TokenType::Section, value};
        }
    }

    // Identifier, true, false
    if (isalpha(currentChar) || currentChar == '_') {
        size_t start = position;
        while (position < input.length() && (isalnum(input[position]) || input[position] == '_')) {
            position++;
        }
        std::string value = input.substr(start, position - start);
        if (value == "true" || value == "false") {
            return {TokenType::Boolean, value};
        }
        return {TokenType::Identifier, value};
    }

    // Equals: =
    if (currentChar == '=') {
        position++;
        return {TokenType::Equals, "="};
    }

    // String: "value"
    if (currentChar == '"') {
        size_t start = ++position;
        while (position < input.length() && input[position] != '"') {
            position++;
        }
        if (position < input.length()) {
            std::string value = input.substr(start, position - start);
            position++; // Skip closing '"'
            return {TokenType::String, value};
        }
    }

    // Number: 123, 3.14
    if (isdigit(currentChar) || (currentChar == '.' && position + 1 < input.length() && isdigit(input[position+1]))) {
        size_t start = position;
        while (position < input.length() && (isdigit(input[position]) || input[position] == '.')) {
            position++;
        }
        return {TokenType::Number, input.substr(start, position - start)};
    }

    // If no token is matched, return unexpected
    return {TokenType::Unexpected, std::string(1, input[position++])};
}