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
        Parser(const std::string& content, YiniDocument& document, const std::string& basePath = ".");
        void parse();

    private:
        void parseSection();
        void parseKeyValuePair(YiniSection& section);
        void parseQuickRegistration(YiniSection& section);
        YiniValue parseValue();
        std::unique_ptr<YiniArray> parseArray();
        YiniValue parseExpression();
        YiniValue parseTerm();
        YiniValue parseFactor();

        Lexer m_lexer;
        Token m_currentToken;
        YiniDocument& m_document;
        std::string m_basePath;
        void nextToken();
    };
}

#endif // YINI_PARSER_HPP