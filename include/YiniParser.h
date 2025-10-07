#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include "YiniValue.h"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <set>

class YiniParser {
public:
    YiniParser();

    void parse(const std::string& content);

    std::optional<YiniValue> getValue(const std::string& section, const std::string& key) const;

private:
    enum class ParserState {
        Idle,
        ParsingSectionName,
        ParsingKey,
        ParsingValue,
        LineComment,
        BlockComment,
        ParsingArray,
        ParsingMap
    };

    void processChar(char c);
    void processSectionHeader(const std::string& header);
    void commitValue();
    void resolveReferences();
    YiniValue resolveValue(const YiniValue& value, std::set<std::string>& visited) const;

    ParserState state = ParserState::Idle;
    std::map<std::string, std::map<std::string, YiniValue>> data;
    std::map<std::string, YiniValue> macros;
    std::map<std::string, std::vector<std::string>> inheritance;
    std::map<std::string, int> quickValueCounters;
    std::string currentSection;
    std::string currentKey;
    std::string currentValue;
    char previousChar = '\0';
    int containerNestingLevel = 0;
};

#endif //YINI_PARSER_H