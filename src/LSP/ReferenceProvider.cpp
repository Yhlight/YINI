#include "LSP/ReferenceProvider.h"
#include <sstream>
#include <cctype>

namespace yini::lsp
{

ReferenceProvider::ReferenceProvider() {}

std::string ReferenceProvider::getLineAtPosition(const std::string& content, int line)
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

std::string ReferenceProvider::getWordAtPosition(const std::string& content, Position pos)
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

bool ReferenceProvider::isMacroReference(const std::string& line, int character)
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

bool ReferenceProvider::isCrossSectionReference(const std::string& line, int character)
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

json ReferenceProvider::makeLocation(const std::string& uri, int line, int startChar, int endChar)
{
    return {
        {"uri", uri},
        {"range", {
            {"start", {{"line", line}, {"character", startChar}}},
            {"end", {{"line", line}, {"character", endChar}}}
        }}
    };
}

json ReferenceProvider::findMacroReferences(
    const std::string& content,
    const std::string& uri,
    const std::string& macroName,
    bool includeDeclaration)
{
    json locations = json::array();
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    bool in_define_section = false;

    while (std::getline(stream, line))
    {
        if (line.find("[#define]") != std::string::npos)
        {
            in_define_section = true;
        }
        else if (in_define_section && line.find('[') != std::string::npos)
        {
            in_define_section = false;
        }

        if (in_define_section && includeDeclaration)
        {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string key = line.substr(0, equals_pos);
                size_t start = key.find_first_not_of(" \t");
                if (start != std::string::npos)
                {
                    size_t end = key.find_last_not_of(" \t");
                    key = key.substr(start, end - start + 1);
                    if (key == macroName)
                    {
                        locations.push_back(makeLocation(uri, line_num, static_cast<int>(start), static_cast<int>(end + 1)));
                    }
                }
            }
        }

        std::string pattern = "@" + macroName;
        size_t pos = 0;
        while ((pos = line.find(pattern, pos)) != std::string::npos)
        {
            if (pos + pattern.length() >= line.length() || !isalnum(line[pos + pattern.length()]))
            {
                locations.push_back(makeLocation(uri, line_num, static_cast<int>(pos), static_cast<int>(pos + pattern.length())));
            }
            pos += pattern.length();
        }
        line_num++;
    }
    return locations;
}

json ReferenceProvider::findKeyReferences(
    const std::string& content,
    const std::string& uri,
    const std::string& section,
    const std::string& key,
    bool includeDeclaration)
{
    json locations = json::array();
    std::istringstream stream(content);
    std::string line;
    int line_num = 0;
    bool in_target_section = false;
    std::string section_header = "[" + section + "]";

    while (std::getline(stream, line))
    {
        if (line.find(section_header) != std::string::npos)
        {
            in_target_section = true;
        }
        else if (in_target_section && line.find('[') != std::string::npos)
        {
            in_target_section = false;
        }

        if (in_target_section && includeDeclaration)
        {
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos)
            {
                std::string found_key = line.substr(0, equals_pos);
                size_t start = found_key.find_first_not_of(" \t");
                if (start != std::string::npos)
                {
                    size_t end = found_key.find_last_not_of(" \t");
                    found_key = found_key.substr(start, end - start + 1);
                    if (found_key == key)
                    {
                        locations.push_back(makeLocation(uri, line_num, static_cast<int>(start), static_cast<int>(end + 1)));
                    }
                }
            }
        }

        std::string pattern = "@{" + section + "." + key + "}";
        size_t pos = 0;
        while ((pos = line.find(pattern, pos)) != std::string::npos)
        {
            locations.push_back(makeLocation(uri, line_num, static_cast<int>(pos), static_cast<int>(pos + pattern.length())));
            pos += pattern.length();
        }
        line_num++;
    }
    return locations;
}

json ReferenceProvider::findReferences(
    yini::Interpreter* /*interpreter*/,
    Document* document,
    const std::string& uri,
    Position position,
    bool includeDeclaration)
{
    const std::string& content = document->content;
    std::string line = getLineAtPosition(content, position.line);
    if (line.empty())
    {
        return json::array();
    }

    if (isMacroReference(line, position.character))
    {
        std::string word = getWordAtPosition(content, position);
        if (!word.empty())
        {
            return findMacroReferences(content, uri, word, includeDeclaration);
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
                    return findKeyReferences(content, uri, section, key, includeDeclaration);
                }
            }
        }
    }

    std::string word = getWordAtPosition(content, position);
    if (!word.empty())
    {
        json macroRefs = findMacroReferences(content, uri, word, includeDeclaration);
        if (!macroRefs.empty())
        {
            return macroRefs;
        }
    }

    return json::array();
}

} // namespace yini::lsp