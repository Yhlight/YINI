#pragma once

#include <string>
#include "Lexer/Token.h"

namespace YINI
{
    class Lexer
    {
    public:
        Lexer(const std::string& input);
        Token nextToken();

    private:
        void readChar();
        char peekChar();
        void skip_whitespace();
        void skip_single_line_comment();
        void skip_multi_line_comment();
        std::string read_identifier();
        std::string read_string();
        Token read_number();

        std::string m_input;
        size_t m_position;      // current position in input (points to current char)
        size_t m_readPosition;  // current reading position in input (after current char)
        char m_char;            // current char under examination
        int m_line;
        int m_column;
    };
}