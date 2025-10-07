#include "YiniParser.h"
#include <algorithm>
#include <sstream>
#include <deque>
#include <set>
#include <cctype>

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper function to parse a value string into a YiniValue
static YiniValue parseValue(const std::string& valueStr) {
    std::string trimmedValue = trim(valueStr);
    if (trimmedValue == "true") return true;
    if (trimmedValue == "false") return false;

    try {
        size_t pos;
        int i_val = std::stoi(trimmedValue, &pos);
        if (pos == trimmedValue.length()) return i_val;
    } catch (...) {}

    try {
        size_t pos;
        double d_val = std::stod(trimmedValue, &pos);
        if (pos == trimmedValue.length()) return d_val;
    } catch (...) {}

    // If value is wrapped in quotes, it's a string
    if (trimmedValue.length() >= 2 && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
        return trimmedValue.substr(1, trimmedValue.length() - 2);
    }

    return trimmedValue;
}

void YiniParser::processSectionHeader(const std::string& header) {
    std::string trimmedHeader = trim(header);
    size_t colonPos = trimmedHeader.find(':');

    std::string childName;
    if (colonPos != std::string::npos) {
        childName = trim(trimmedHeader.substr(0, colonPos));
        std::string parentsStr = trim(trimmedHeader.substr(colonPos + 1));

        std::stringstream ss(parentsStr);
        std::string parent;
        while (std::getline(ss, parent, ',')) {
            inheritance[childName].push_back(trim(parent));
        }
    } else {
        childName = trimmedHeader;
    }
    currentSection = childName;
}


YiniParser::YiniParser() {}

void YiniParser::parse(const std::string& content) {
    // Reset state for new parsing
    data.clear();
    inheritance.clear();
    quickValueCounters.clear();
    state = ParserState::Idle;
    currentSection.clear();
    currentKey.clear();
    currentValue.clear();
    previousChar = '\0';

    std::string processed_content = content + "\n";

    for (char c : processed_content) {
        processChar(c);
    }
}

void YiniParser::processChar(char c) {
    // Comment handling should be checked first, as it can interrupt other states.
    if (previousChar == '/') {
        if (c == '/') {
            // Start of a line comment.
            if (state == ParserState::ParsingValue) {
                currentValue.pop_back(); // remove the extraneous '/'
                // Commit the value parsed so far.
                if (!currentSection.empty() && !currentKey.empty()) {
                    std::string key = trim(currentKey);
                    data[currentSection][key] = parseValue(currentValue);
                }
                currentKey.clear();
                currentValue.clear();
            } else if (state == ParserState::ParsingKey) {
                currentKey.pop_back(); // remove the extraneous '/'
                currentKey.clear();
            }
            state = ParserState::LineComment;
            previousChar = c;
            return;
        } else if (c == '*') {
            // Start of a block comment.
            if (state == ParserState::ParsingValue) {
                currentValue.pop_back(); // remove the extraneous '/'
                // Commit the value parsed so far.
                if (!currentSection.empty() && !currentKey.empty()) {
                    std::string key = trim(currentKey);
                    data[currentSection][key] = parseValue(currentValue);
                }
                currentKey.clear();
                currentValue.clear();
            } else if (state == ParserState::ParsingKey) {
                currentKey.pop_back(); // remove the extraneous '/'
                currentKey.clear();
            }
            state = ParserState::BlockComment;
            previousChar = c;
            return;
        }
    }

    switch (state) {
        case ParserState::Idle:
            if (c == '[') {
                state = ParserState::ParsingSectionName;
                currentSection.clear();
            } else if (!isspace(c)) {
                state = ParserState::ParsingKey;
                currentKey.clear();
                currentKey += c;
            }
            break;

        case ParserState::ParsingSectionName:
            if (c == ']') {
                processSectionHeader(currentSection);
                state = ParserState::Idle;
            } else {
                currentSection += c;
            }
            break;

        case ParserState::ParsingKey:
            if (c == '=') {
                if (currentKey == "+") {
                    // This is a += operation
                    int index = quickValueCounters[currentSection]++;
                    currentKey = std::to_string(index);
                }
                state = ParserState::ParsingValue;
                currentValue.clear();
            } else if (c == '\n' || c == '\r') {
                state = ParserState::Idle;
            }
            else {
                currentKey += c;
            }
            break;

        case ParserState::ParsingValue:
            if (c == '\n' || c == '\r') {
                if (!currentSection.empty() && !currentKey.empty()) {
                    std::string key = trim(currentKey);
                    data[currentSection][key] = parseValue(currentValue);
                }
                currentKey.clear();
                currentValue.clear();
                state = ParserState::Idle;
            } else {
                currentValue += c;
            }
            break;

        case ParserState::LineComment:
            if (c == '\n' || c == '\r') {
                state = ParserState::Idle;
            }
            break;

        case ParserState::BlockComment:
            if (previousChar == '*' && c == '/') {
                state = ParserState::Idle;
            }
            break;
    }
    previousChar = c;
}

std::optional<YiniValue> YiniParser::getValue(const std::string& section, const std::string& key) const {
    std::deque<std::string> sectionsToCheck;
    sectionsToCheck.push_back(section);
    std::set<std::string> visitedSections;

    while (!sectionsToCheck.empty()) {
        std::string currentSectionName = sectionsToCheck.front();
        sectionsToCheck.pop_front();

        if (visitedSections.count(currentSectionName)) {
            continue; // Cycle detected
        }
        visitedSections.insert(currentSectionName);

        auto sectionIt = data.find(currentSectionName);
        if (sectionIt != data.end()) {
            auto keyIt = sectionIt->second.find(key);
            if (keyIt != sectionIt->second.end()) {
                return keyIt->second;
            }
        }

        auto inheritanceIt = inheritance.find(currentSectionName);
        if (inheritanceIt != inheritance.end()) {
            const auto& parents = inheritanceIt->second;
            // For [C: P1, P2], parents is ["P1", "P2"]. We want to check P2 then P1.
            // To make the queue [P2, P1, ...], we must prepend P1, then P2.
            // So we iterate through the parents list forwards.
            for (const auto& parent : parents) {
                 sectionsToCheck.push_front(parent);
            }
        }
    }

    return std::nullopt;
}