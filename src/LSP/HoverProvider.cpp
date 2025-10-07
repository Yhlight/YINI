#include "LSP/HoverProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

HoverProvider::HoverProvider() {}

std::string HoverProvider::getLineAtPosition(const std::string& content, int line)
{
    std::istringstream stream(content);
    std::string current_line;
    int current = 0;
    while (std::getline(stream, current_line))
    {
        if (current == line)
        {
            return current_line;
        }
        current++;
    }
    return "";
}

std::string HoverProvider::getWordAtPosition(const std::string& content, Position pos)
{
    std::string line = getLineAtPosition(content, pos.line);
    if (line.empty() || pos.character >= static_cast<int>(line.length()))
    {
        return "";
    }

    int start = pos.character;
    int end = pos.character;

    while (start > 0 && (std::isalnum(line[start - 1]) || line[start - 1] == '_'))
    {
        start--;
    }

    while (end < static_cast<int>(line.length()) && (std::isalnum(line[end]) || line[end] == '_'))
    {
        end++;
    }

    if (start < end)
    {
        return line.substr(start, end - start);
    }
    return "";
}

bool HoverProvider::isMacroReference(const std::string& line, int character)
{
    for (int i = character - 1; i >= 0; i--)
    {
        if (line[i] == '@')
        {
            if (i + 1 < static_cast<int>(line.length()) && line[i + 1] == '{')
            {
                return false;
            }
            return true;
        }
        if (!std::isalnum(line[i]) && line[i] != '_')
        {
            break;
        }
    }
    return false;
}

bool HoverProvider::isCrossSectionReference(const std::string& line, int character)
{
    int brace_count = 0;
    for (int i = 0; i < character && i < static_cast<int>(line.length()); i++)
    {
        if (i + 1 < static_cast<int>(line.length()) && line[i] == '@' && line[i + 1] == '{')
        {
            brace_count++;
            i++;
        }
        else if (line[i] == '}')
        {
            brace_count--;
        }
    }
    return brace_count > 0;
}

std::string HoverProvider::getValueTypeString(std::shared_ptr<yini::Value> value)
{
    if (!value) return "unknown";
    switch (value->getType())
    {
        case yini::ValueType::INTEGER: return "integer";
        case yini::ValueType::FLOAT: return "float";
        case yini::ValueType::BOOLEAN: return "boolean";
        case yini::ValueType::STRING: return "string";
        case yini::ValueType::ARRAY: return "array";
        case yini::ValueType::MAP: return "map";
        case yini::ValueType::COLOR: return "color";
        case yini::ValueType::COORD: return "coordinate";
        case yini::ValueType::LIST: return "list";
        case yini::ValueType::PATH: return "path";
        case yini::ValueType::DYNAMIC: return "dynamic";
        case yini::ValueType::REFERENCE: return "reference";
        case yini::ValueType::ENV_VAR: return "environment variable";
        default: return "unknown";
    }
}

json HoverProvider::getMacroHover(yini::Interpreter* interpreter, const std::string& name)
{
    if (!interpreter) return nullptr;
    const auto& defines = interpreter->getDefines();
    auto it = defines.find(name);
    if (it == defines.end()) return nullptr;

    auto value = it->second;
    std::string type = getValueTypeString(value);
    std::string valueStr = value->toString();

    std::ostringstream content;
    content << "**Macro**: `@" << name << "`\n\n";
    content << "**Type**: `" << type << "`\n\n";
    content << "**Value**: `" << valueStr << "`\n\n";
    content << "Defined in `[#define]` section";
    return makeHoverContent(content.str());
}

json HoverProvider::getSectionKeyHover(yini::Interpreter* interpreter, const std::string& section, const std::string& key)
{
    if (!interpreter) return nullptr;
    const auto& sections = interpreter->getSections();
    auto it = sections.find(section);
    if (it == sections.end()) return nullptr;

    const auto& entries = it->second.entries;
    auto key_it = entries.find(key);
    if (key_it == entries.end()) return nullptr;

    auto value = key_it->second;
    std::string type = getValueTypeString(value);
    std::string valueStr = value->toString();

    std::ostringstream content;
    content << "**Section**: `[" << section << "]`\n\n";
    content << "**Key**: `" << key << "`\n\n";
    content << "**Type**: `" << type << "`\n\n";
    content << "**Value**: `" << valueStr << "`";
    return makeHoverContent(content.str());
}

json HoverProvider::makeHoverContent(const std::string& content, const std::string& /*language*/)
{
    return {{"contents", {{"kind", "markdown"}, {"value", content}}}};
}

json HoverProvider::getHover(
    yini::Interpreter* interpreter,
    Document* document,
    Position position)
{
    const std::string& content = document->content;
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty()) return nullptr;

    if (isMacroReference(line, position.character))
    {
        std::string word = getWordAtPosition(content, position);
        if (!word.empty())
        {
            return getMacroHover(interpreter, word);
        }
    }

    if (isCrossSectionReference(line, position.character))
    {
        size_t at_brace = line.rfind("@{", position.character);
        if (at_brace != std::string::npos)
        {
            size_t close_brace = line.find("}", at_brace);
            if (close_brace != std::string::npos)
            {
                std::string ref = line.substr(at_brace + 2, close_brace - at_brace - 2);
                size_t dot_pos = ref.find('.');
                if (dot_pos != std::string::npos)
                {
                    std::string section = ref.substr(0, dot_pos);
                    std::string key = ref.substr(dot_pos + 1);
                    return getSectionKeyHover(interpreter, section, key);
                }
            }
        }
    }

    std::string word = getWordAtPosition(content, position);
    if (!word.empty() && interpreter)
    {
        const auto& sections = interpreter->getSections();
        for (const auto& [section_name, section] : sections)
        {
            if (section.entries.count(word))
            {
                return getSectionKeyHover(interpreter, section_name, word);
            }
        }
        
        const auto& defines = interpreter->getDefines();
        if (defines.count(word))
        {
            return getMacroHover(interpreter, word);
        }
    }
    
    return nullptr;
}

} // namespace yini::lsp