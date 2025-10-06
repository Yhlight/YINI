#include "Token.h"
#include <sstream>

namespace yini
{

std::string Token::toString() const
{
    std::ostringstream oss;
    oss << "Token(";
    
    // Token type
    switch (type)
    {
        case TokenType::END_OF_FILE:    oss << "EOF"; break;
        case TokenType::NEWLINE:        oss << "NEWLINE"; break;
        case TokenType::INTEGER:        oss << "INTEGER"; break;
        case TokenType::FLOAT:          oss << "FLOAT"; break;
        case TokenType::BOOLEAN:        oss << "BOOLEAN"; break;
        case TokenType::STRING:         oss << "STRING"; break;
        case TokenType::IDENTIFIER:     oss << "IDENTIFIER"; break;
        case TokenType::SECTION_START:  oss << "SECTION_START"; break;
        case TokenType::SECTION_END:    oss << "SECTION_END"; break;
        case TokenType::COLON:          oss << "COLON"; break;
        case TokenType::EQUALS:         oss << "EQUALS"; break;
        case TokenType::PLUS_EQUALS:    oss << "PLUS_EQUALS"; break;
        case TokenType::PLUS:           oss << "PLUS"; break;
        case TokenType::MINUS:          oss << "MINUS"; break;
        case TokenType::MULTIPLY:       oss << "MULTIPLY"; break;
        case TokenType::DIVIDE:         oss << "DIVIDE"; break;
        case TokenType::MODULO:         oss << "MODULO"; break;
        case TokenType::COMMA:          oss << "COMMA"; break;
        case TokenType::LPAREN:         oss << "LPAREN"; break;
        case TokenType::RPAREN:         oss << "RPAREN"; break;
        case TokenType::LBRACKET:       oss << "LBRACKET"; break;
        case TokenType::RBRACKET:       oss << "RBRACKET"; break;
        case TokenType::LBRACE:         oss << "LBRACE"; break;
        case TokenType::RBRACE:         oss << "RBRACE"; break;
        case TokenType::AT:             oss << "AT"; break;
        case TokenType::DOLLAR_LBRACE:  oss << "DOLLAR_LBRACE"; break;
        case TokenType::AT_LBRACE:      oss << "AT_LBRACE"; break;
        case TokenType::HASH:           oss << "HASH"; break;
        case TokenType::EXCLAMATION:    oss << "EXCLAMATION"; break;
        case TokenType::QUESTION:       oss << "QUESTION"; break;
        case TokenType::TILDE:          oss << "TILDE"; break;
        case TokenType::COLOR:          oss << "COLOR"; break;
        case TokenType::COORD:          oss << "COORD"; break;
        case TokenType::PATH:           oss << "PATH"; break;
        case TokenType::LIST:           oss << "LIST"; break;
        case TokenType::ARRAY:          oss << "ARRAY"; break;
        case TokenType::MAP:            oss << "MAP"; break;
        case TokenType::DYNA:           oss << "DYNA"; break;
        case TokenType::COMMENT:        oss << "COMMENT"; break;
        case TokenType::ERROR:          oss << "ERROR"; break;
        default:                        oss << "UNKNOWN"; break;
    }
    
    // Token value
    if (hasValue())
    {
        oss << ", value=";
        if (std::holds_alternative<int64_t>(value))
        {
            oss << std::get<int64_t>(value);
        }
        else if (std::holds_alternative<double>(value))
        {
            oss << std::get<double>(value);
        }
        else if (std::holds_alternative<bool>(value))
        {
            oss << (std::get<bool>(value) ? "true" : "false");
        }
        else if (std::holds_alternative<std::string>(value))
        {
            oss << "\"" << std::get<std::string>(value) << "\"";
        }
    }
    
    oss << ", line=" << line << ", col=" << column << ")";
    return oss.str();
}

} // namespace yini
