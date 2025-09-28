#ifndef YINI_PARSER_HPP
#define YINI_PARSER_HPP

#include "YiniData.hpp"
#include "Lexer.hpp"
#include <string>
#include <memory>

namespace YINI
{
    class Parser
    {
    public:
        Parser(const std::string& content);
        YiniDocument parse();

    private:
        void parseSection(YiniSection& section);
        void parseKeyValuePair(YiniSection& section);
        void parseQuickRegistration(YiniSection& section);
        YiniValue parseValue();
        std::unique_ptr<YiniArray> parseArray();

        Lexer m_lexer;
        Token m_currentToken;
        void nextToken();
    };
}

#endif // YINI_PARSER_HPP