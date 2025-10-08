#include "parser.h"
#include <stdexcept>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    nextToken();
}

void Parser::nextToken() {
    currentToken = lexer.nextToken();
}

Config Parser::parse() {
    Config config;
    ConfigSection* currentSection = nullptr;

    while (currentToken.type != TokenType::EndOfFile) {
        if (currentToken.type == TokenType::Section) {
            currentSection = &config[currentToken.value];
            nextToken();
        } else if (currentToken.type == TokenType::Identifier) {
            if (!currentSection) {
                throw std::runtime_error("Key-value pair outside of a section");
            }

            std::string key = currentToken.value;
            nextToken(); // Consume identifier

            if (currentToken.type != TokenType::Equals) {
                throw std::runtime_error("Expected '=' after key");
            }
            nextToken(); // Consume '='

            if (currentToken.type == TokenType::String) {
                (*currentSection)[key] = currentToken.value;
            } else if (currentToken.type == TokenType::Number) {
                if (currentToken.value.find('.') != std::string::npos) {
                    (*currentSection)[key] = std::stod(currentToken.value);
                } else {
                    (*currentSection)[key] = std::stoi(currentToken.value);
                }
            } else if (currentToken.type == TokenType::Boolean) {
                (*currentSection)[key] = (currentToken.value == "true");
            }
            else {
                throw std::runtime_error("Expected a value (string, number, or boolean)");
            }

            nextToken(); // Consume value
        } else {
            nextToken();
        }
    }

    return config;
}