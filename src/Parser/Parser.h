#pragma once

#include "../Lexer/Lexer.h"
#include "AST.h"
#include <memory>
#include <vector>

namespace YINI
{
    class Parser
    {
    public:
        Parser(Lexer& lexer);

        std::unique_ptr<YiniFile> parse();

    private:
        void advance();
        bool match(TokenType type);
        void consume(TokenType type, const std::string& message);
        Token peek() const;

        std::unique_ptr<Section> parseSection();
        std::unique_ptr<KeyValuePair> parseKeyValuePair();
        std::unique_ptr<Value> parseValue();
        std::unique_ptr<Value> parseArray();
        std::unique_ptr<Value> parseMap();
        std::unique_ptr<Value> parseCoordinate();

        Lexer& m_lexer;
        Token m_current_token;
        std::vector<Token> m_tokens;
        size_t m_token_pos = 0;
    };
}
