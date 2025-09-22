#pragma once

#include "Lexer.h"
#include "YiniData.h"
#include "Token.h"
#include <vector>
#include <string>

namespace Yini
{
    class Parser
    {
    public:
        Parser(Lexer& lexer);

        YiniData parseYini();
        const std::vector<std::string>& getErrors() const;

    private:
        void nextToken();
        bool expectPeek(TokenType t);

        YiniSection* parseSection(YiniData& data);
        void parseKeyValuePair(YiniSection& section, YiniData& data);
        void parsePlusEqual(YiniSection& section, YiniData& data);

        // Value parsing
        YiniValue parseValue(YiniData& data);
        YiniValue parseArray(YiniData& data);
        YiniValue parseCoordinate(YiniData& data);
        YiniValue parseMap(YiniData& data);
        YiniValue parseColor(YiniData& data);


        Lexer& m_lexer;
        Token m_curToken;
        Token m_peekToken;

        std::vector<std::string> m_errors;
    };
}
