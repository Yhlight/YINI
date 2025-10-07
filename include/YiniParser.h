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
    std::map<std::string, std::map<std::string, YiniValue>> data;
};

#endif //YINI_PARSER_H