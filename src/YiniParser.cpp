#include "YiniParser.h"
#include <algorithm>
#include <sstream>
#include <deque>
#include <set>
#include <cctype>
#include <regex>

// Forward declarations for recursive parsing
static YiniValue parseValue(const std::string& valueStr, const std::map<std::string, YiniValue>& macros);
static YiniArray parseArray(const std::string& arrayStr, const std::map<std::string, YiniValue>& macros);
static YiniList parseList(const std::string& listStr, const std::map<std::string, YiniValue>& macros);
static YiniMap parseMap(const std::string& mapStr, const std::map<std::string, YiniValue>& macros);
static std::optional<YiniColor> parseColor(const std::string& colorStr);
static std::optional<YiniCoord> parseCoord(const std::string& coordStr);
static std::optional<YiniPath> parsePath(const std::string& pathStr);


// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// --- Container Parsers ---

YiniArray parseArray(const std::string& arrayStr, const std::map<std::string, YiniValue>& macros) {
    YiniArray result;
    std::string content = trim(arrayStr);
    if (content.length() < 2 || content.front() != '[' || content.back() != ']') {
        return result;
    }
    content = content.substr(1, content.length() - 2);

    std::string element;
    int nestingLevel = 0;
    bool inQuotes = false;

    for (char c : content) {
        if (c == '"') inQuotes = !inQuotes;
        if (!inQuotes) {
            if (c == '[' || c == '{') nestingLevel++;
            if (c == ']' || c == '}') nestingLevel--;
        }

        if (c == ',' && nestingLevel == 0 && !inQuotes) {
            result.push_back(parseValue(element, macros));
            element.clear();
        } else {
            element += c;
        }
    }
    if (!element.empty()) {
        result.push_back(parseValue(element, macros));
    }
    return result;
}

YiniList parseList(const std::string& listStr, const std::map<std::string, YiniValue>& macros) {
    YiniList result;
    std::string content = trim(listStr);
    if (content.length() < 6 || (content.rfind("List(", 0) != 0 && content.rfind("list(", 0) != 0) || content.back() != ')') {
        return result;
    }
    content = content.substr(5, content.length() - 6);

    std::string element;
    int nestingLevel = 0;
    bool inQuotes = false;

    for (char c : content) {
        if (c == '"') inQuotes = !inQuotes;
        if (!inQuotes) {
            if (c == '[' || c == '{' || c == '(') nestingLevel++;
            if (c == ']' || c == '}' || c == ')') nestingLevel--;
        }

        if (c == ',' && nestingLevel == 0 && !inQuotes) {
            result.push_back(parseValue(element, macros));
            element.clear();
        } else {
            element += c;
        }
    }
    if (!element.empty()) {
        result.push_back(parseValue(element, macros));
    }
    return result;
}

YiniMap parseMap(const std::string& mapStr, const std::map<std::string, YiniValue>& macros) {
    YiniMap result;
    std::string content = trim(mapStr);
    if (content.length() < 2 || content.front() != '{' || content.back() != '}') {
        return result;
    }
    content = content.substr(1, content.length() - 2);

    std::string pair;
    int nestingLevel = 0;
    bool inQuotes = false;

    for (char c : content) {
        if (c == '"') inQuotes = !inQuotes;
        if (!inQuotes) {
            if (c == '[' || c == '{') nestingLevel++;
            if (c == ']' || c == '}') nestingLevel--;
        }

        if (c == ',' && nestingLevel == 0 && !inQuotes) {
            size_t colonPos = pair.find(':');
            if (colonPos != std::string::npos) {
                std::string key = trim(pair.substr(0, colonPos));
                std::string valStr = trim(pair.substr(colonPos + 1));
                result[key] = parseValue(valStr, macros);
            }
            pair.clear();
        } else {
            pair += c;
        }
    }
    if (!pair.empty()) {
        size_t colonPos = pair.find(':');
        if (colonPos != std::string::npos) {
            std::string key = trim(pair.substr(0, colonPos));
            std::string valStr = trim(pair.substr(colonPos + 1));
            result[key] = parseValue(valStr, macros);
        }
    }

    return result;
}


// --- Type-specific Parsers ---

std::optional<YiniColor> parseColor(const std::string& colorStr) {
    std::string trimmed = trim(colorStr);
    if (trimmed.empty()) return std::nullopt;

    // Hex format: #RRGGBB
    if (trimmed.front() == '#') {
        if (trimmed.length() != 7) return std::nullopt;
        try {
            uint8_t r = std::stoul(trimmed.substr(1, 2), nullptr, 16);
            uint8_t g = std::stoul(trimmed.substr(3, 2), nullptr, 16);
            uint8_t b = std::stoul(trimmed.substr(5, 2), nullptr, 16);
            return YiniColor{r, g, b, 255};
        } catch (...) {
            return std::nullopt;
        }
    }

    // Functional format: color(r, g, b)
    static const std::regex color_regex(R"((?:color|Color)\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\))");
    std::smatch match;
    if (std::regex_match(trimmed, match, color_regex)) {
        try {
            uint8_t r = std::stoi(match[1]);
            uint8_t g = std::stoi(match[2]);
            uint8_t b = std::stoi(match[3]);
            return YiniColor{r, g, b, 255};
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<YiniCoord> parseCoord(const std::string& coordStr) {
    std::string trimmed = trim(coordStr);
    static const std::regex coord_regex(R"((?:Coord|coord)\s*\(\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)(?:\s*,\s*([+-]?\d*\.?\d+))?\s*\))");
    std::smatch match;

    if (std::regex_match(trimmed, match, coord_regex)) {
        try {
            double x = std::stod(match[1]);
            double y = std::stod(match[2]);
            if (match[3].matched) { // 3D
                double z = std::stod(match[3]);
                return YiniCoord{x, y, z, true};
            } else { // 2D
                return YiniCoord{x, y, 0.0, false};
            }
        } catch (...) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<YiniPath> parsePath(const std::string& pathStr) {
    std::string trimmed = trim(pathStr);
    static const std::regex path_regex(R"((?:Path|path)\s*\(\s*\"(.*)\"\s*\))");
    std::smatch match;
    if (std::regex_match(trimmed, match, path_regex)) {
        return YiniPath{match[1].str()};
    }
    return std::nullopt;
}


// --- Main Value Parser ---
static YiniValue parseValue(const std::string& valueStr, const std::map<std::string, YiniValue>& macros) {
    std::string trimmedValue = trim(valueStr);

    // Check for macro reference
    if (!trimmedValue.empty() && trimmedValue.front() == '@' && trimmedValue.find('{') == std::string::npos) {
        std::string macroName = trimmedValue.substr(1);
        auto it = macros.find(macroName);
        if (it != macros.end()) {
            return it->second;
        }
    }

    // Check for explicit types first
    if (auto color = parseColor(trimmedValue); color.has_value()) return YiniValue(color.value());
    if (auto coord = parseCoord(trimmedValue); coord.has_value()) return YiniValue(coord.value());
    if (auto path = parsePath(trimmedValue); path.has_value()) return YiniValue(path.value());

    if ((trimmedValue.rfind("List(", 0) == 0 || trimmedValue.rfind("list(", 0) == 0)) {
        return YiniValue(parseList(trimmedValue, macros));
    }


    // Check for containers
    if (!trimmedValue.empty() && trimmedValue.front() == '[' && trimmedValue.back() == ']') {
        return YiniValue(parseArray(trimmedValue, macros));
    }
    if (!trimmedValue.empty() && trimmedValue.front() == '{' && trimmedValue.back() == '}') {
        return YiniValue(parseMap(trimmedValue, macros));
    }

    // Fallback to primitive types
    if (trimmedValue == "true") return YiniValue(true);
    if (trimmedValue == "false") return YiniValue(false);

    try {
        size_t pos;
        int i_val = std::stoi(trimmedValue, &pos);
        if (pos == trimmedValue.length()) return YiniValue(i_val);
    } catch (...) {}

    try {
        size_t pos;
        double d_val = std::stod(trimmedValue, &pos);
        if (pos == trimmedValue.length()) return YiniValue(d_val);
    } catch (...) {}

    // If value is wrapped in quotes, it's a string
    if (trimmedValue.length() >= 2 && trimmedValue.front() == '"' && trimmedValue.back() == '"') {
        return YiniValue(trimmedValue.substr(1, trimmedValue.length() - 2));
    }

    return YiniValue(trimmedValue);
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

void YiniParser::commitValue() {
    if (currentSection.empty() || currentKey.empty()) {
        return;
    }

    std::string key = trim(currentKey);
    // Pass the current state of macros to the parser
    YiniValue value = parseValue(currentValue, macros);

    if (currentSection == "#define") {
        macros[key] = value;
    } else {
        data[currentSection][key] = value;
    }

    currentKey.clear();
    currentValue.clear();
}


YiniParser::YiniParser() {}

void YiniParser::parse(const std::string& content) {
    // Reset state for new parsing
    data.clear();
    macros.clear();
    inheritance.clear();
    quickValueCounters.clear();
    state = ParserState::Idle;
    currentSection.clear();
    currentKey.clear();
    currentValue.clear();
    previousChar = '\0';
    containerNestingLevel = 0;

    std::string processed_content = content + "\n";

    for (char c : processed_content) {
        processChar(c);
    }

    resolveReferences();
}

void YiniParser::processChar(char c) {
    // Comment handling should be checked first, as it can interrupt other states.
    if (previousChar == '/') {
        if (c == '/') {
            if (state == ParserState::ParsingValue) {
                currentValue.pop_back();
                commitValue();
            } else if (state == ParserState::ParsingKey) {
                currentKey.pop_back(); currentKey.clear();
            }
            state = ParserState::LineComment;
            previousChar = c;
            return;
        } else if (c == '*') {
            if (state == ParserState::ParsingValue) {
                currentValue.pop_back();
                commitValue();
            } else if (state == ParserState::ParsingKey) {
                currentKey.pop_back(); currentKey.clear();
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
                if (trim(currentKey) == "+") {
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
            if (currentValue.empty() && isspace(c)) {
                 break; // Ignore leading whitespace
            }
            if (currentValue.empty()) {
                if (c == '[') {
                    state = ParserState::ParsingArray;
                    containerNestingLevel = 1;
                } else if (c == '{') {
                    state = ParserState::ParsingMap;
                    containerNestingLevel = 1;
                }
            }
            if (state == ParserState::ParsingValue) { // If not transitioned to container
                if (c == '\n' || c == '\r') {
                    commitValue();
                    state = ParserState::Idle;
                }
            }
            currentValue += c; // Always append char if we are in a value-parsing state
            break;

        case ParserState::ParsingArray:
            currentValue += c;
            if (c == '[') containerNestingLevel++;
            if (c == ']') containerNestingLevel--;
            if (containerNestingLevel == 0) {
                commitValue();
                state = ParserState::Idle;
            }
            break;

        case ParserState::ParsingMap:
            currentValue += c;
            if (c == '{') containerNestingLevel++;
            if (c == '}') containerNestingLevel--;
            if (containerNestingLevel == 0) {
                commitValue();
                state = ParserState::Idle;
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

YiniValue YiniParser::resolveValue(const YiniValue& value, std::set<std::string>& visited) const {
    if (!value.is<std::string>()) {
        return value; // Not a string, nothing to resolve
    }

    const std::string& str = value.get<std::string>();
    static const std::regex ref_regex(R"(@\{([^}]+)\})");
    std::smatch match;

    if (std::regex_match(str, match, ref_regex)) {
        std::string fullKey = match[1].str();

        if (visited.count(fullKey)) {
            // Cycle detected, return as-is to prevent infinite loop
            return value;
        }
        visited.insert(fullKey);

        size_t dotPos = fullKey.find('.');
        if (dotPos != std::string::npos) {
            std::string sectionName = fullKey.substr(0, dotPos);
            std::string keyName = fullKey.substr(dotPos + 1);

            auto resolved_value = getValue(sectionName, keyName);
            if(resolved_value.has_value()){
                 return resolveValue(resolved_value.value(), visited);
            }
        }
        visited.erase(fullKey);
    }

    return value;
}

void YiniParser::resolveReferences() {
    for (auto& section_pair : data) {
        for (auto& key_pair : section_pair.second) {
            std::set<std::string> visited;
            key_pair.second = resolveValue(key_pair.second, visited);
        }
    }
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
                 std::set<std::string> visitedRefs;
                 return resolveValue(keyIt->second, visitedRefs);
            }
        }

        auto inheritanceIt = inheritance.find(currentSectionName);
        if (inheritanceIt != inheritance.end()) {
            const auto& parents = inheritanceIt->second;
            for (const auto& parent : parents) {
                 sectionsToCheck.push_front(parent);
            }
        }
    }

    return std::nullopt;
}