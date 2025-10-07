#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>

// Define the YiniValue type that can hold different data types.
using YiniValue = std::variant<std::string, int, bool, double>;

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
        BlockComment
    };

    void processChar(char c);
    void processSectionHeader(const std::string& header);

    ParserState state = ParserState::Idle;
    std::map<std::string, std::map<std::string, YiniValue>> data;
    std::map<std::string, std::vector<std::string>> inheritance;
    std::map<std::string, int> quickValueCounters;
    std::string currentSection;
    std::string currentKey;
    std::string currentValue;
    char previousChar = '\0';
};

#endif //YINI_PARSER_H