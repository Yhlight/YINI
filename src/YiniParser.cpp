#include "YiniParser.h"
#include <sstream>
#include <algorithm>

YiniParser::YiniParser() {}

// Helper function to trim whitespace from both ends of a string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper function to parse a value string into a YiniValue
static YiniValue parseValue(const std::string& valueStr) {
    // Check for boolean
    if (valueStr == "true") {
        return true;
    }
    if (valueStr == "false") {
        return false;
    }

    // Check for integer
    try {
        size_t pos;
        int val = std::stoi(valueStr, &pos);
        if (pos == valueStr.length()) {
            return val;
        }
    } catch (const std::invalid_argument&) {
        // Not an integer
    } catch (const std::out_of_range&) {
        // Out of range for int
    }

    // Check for double
    try {
        size_t pos;
        double val = std::stod(valueStr, &pos);
        if (pos == valueStr.length()) {
            return val;
        }
    } catch (const std::invalid_argument&) {
        // Not a double
    } catch (const std::out_of_range&) {
        // Out of range for double
    }

    // Fallback to string
    return valueStr;
}


void YiniParser::parse(const std::string& content) {
    std::stringstream ss(content);
    std::string line;
    std::string currentSection;

    while (std::getline(ss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';') { // Skip empty lines and comments
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
        } else {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string valueStr = trim(line.substr(delimiterPos + 1));
                if (!currentSection.empty()) {
                    data[currentSection][key] = parseValue(valueStr);
                }
            }
        }
    }
}

std::optional<YiniValue> YiniParser::getValue(const std::string& section, const std::string& key) const {
    auto sectionIt = data.find(section);
    if (sectionIt != data.end()) {
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            return keyIt->second;
        }
    }
    return std::nullopt;
}