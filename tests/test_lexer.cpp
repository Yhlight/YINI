#include "../src/Lexer/Lexer.h"
#include <iostream>
#include <vector>

// Helper to print token types
std::string tokenTypeToString(YINI::TokenType type) {
    switch (type) {
        case YINI::TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case YINI::TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case YINI::TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case YINI::TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case YINI::TokenType::COMMA: return "COMMA";
        case YINI::TokenType::COLON: return "COLON";
        case YINI::TokenType::EQUAL: return "EQUAL";
        case YINI::TokenType::AT: return "AT";
        case YINI::TokenType::HASH: return "HASH";
        case YINI::TokenType::PLUS_EQUAL: return "PLUS_EQUAL";
        case YINI::TokenType::IDENTIFIER: return "IDENTIFIER";
        case YINI::TokenType::STRING: return "STRING";
        case YINI::TokenType::INTEGER: return "INTEGER";
        case YINI::TokenType::FLOAT: return "FLOAT";
        case YINI::TokenType::BOOLEAN: return "BOOLEAN";
        case YINI::TokenType::COMMENT: return "COMMENT";
        case YINI::TokenType::NEWLINE: return "NEWLINE";
        case YINI::TokenType::END_OF_FILE: return "END_OF_FILE";
        case YINI::TokenType::UNKNOWN: return "UNKNOWN";
        default: return "UNHANDLED_TOKEN";
    }
}


int main() {
    std::string source = R"yini(
// This is a comment
[#define]
name = "YINI"

[#include]
+= "common.yini"

/*
  Multi-line comment
*/
[TestSection] : BaseSection
key1 = 123
key2 = -45.6
is_true = true
name_ref = @name
+= "value1"
)yini";

    YINI::Lexer lexer(source);
    std::vector<YINI::Token> tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token { type: " << tokenTypeToString(token.type)
                  << ", literal: '" << token.literal
                  << "', line: " << token.line
                  << ", col: " << token.column << " }" << std::endl;
    }

    return 0;
}
