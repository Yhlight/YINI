#include "LSP/CompletionProvider.h"
#include <sstream>

namespace yini::lsp
{

CompletionProvider::CompletionProvider() {}

std::string CompletionProvider::getLineAtPosition(const std::string& content, int line)
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

std::string CompletionProvider::getTextBeforeCursor(const std::string& content, Position pos)
{
    std::istringstream stream(content);
    std::string line;
    int current = 0;
    while (std::getline(stream, line))
    {
        if (current == pos.line)
        {
            return line.substr(0, pos.character);
        }
        current++;
    }
    return "";
}

json CompletionProvider::getCompletions(
    yini::Interpreter* interpreter,
    Document* document,
    Position position)
{
    std::string textBefore = getTextBeforeCursor(document->content, position);
    json completions = json::array();

    // Directive completion: [#
    if (textBefore.find("[#") != std::string::npos && textBefore.find("]") == std::string::npos)
    {
        auto directives = completeDirectives("");
        completions.insert(completions.end(), directives.begin(), directives.end());
    }

    // Macro reference completion: @
    size_t at_pos = textBefore.rfind("@");
    if (at_pos != std::string::npos && textBefore.find("{", at_pos) == std::string::npos)
    {
        std::string prefix = textBefore.substr(at_pos + 1);
        auto macros = completeMacroReferences(interpreter, prefix);
        completions.insert(completions.end(), macros.begin(), macros.end());
    }

    // Cross-section reference: @{
    size_t at_brace_pos = textBefore.rfind("@{");
    if (at_brace_pos != std::string::npos)
    {
        std::string prefix = textBefore.substr(at_brace_pos + 2);
        auto sections = completeSectionReferences(interpreter, prefix);
        completions.insert(completions.end(), sections.begin(), sections.end());
    }

    auto types = completeDataTypes("");
    completions.insert(completions.end(), types.begin(), types.end());
    
    auto keywords = completeKeywords("");
    completions.insert(completions.end(), keywords.begin(), keywords.end());

    return completions;
}

json CompletionProvider::completeDirectives(const std::string& /*prefix*/)
{
    json items = json::array();
    items.push_back(makeCompletionItem("[#define]", 14, "Macro definitions", "Define macros for reuse with @name"));
    items.push_back(makeCompletionItem("[#include]", 14, "File includes", "Include other YINI files"));
    items.push_back(makeCompletionItem("[#schema]", 14, "Schema validation", "Define validation rules for sections"));
    return items;
}

json CompletionProvider::completeKeywords(const std::string& /*prefix*/)
{
    json items = json::array();
    items.push_back(makeCompletionItem("true", 12, "Boolean", "Boolean true value"));
    items.push_back(makeCompletionItem("false", 12, "Boolean", "Boolean false value"));
    return items;
}

json CompletionProvider::completeMacroReferences(yini::Interpreter* interpreter, const std::string& /*prefix*/)
{
    json items = json::array();
    if (!interpreter) return items;

    const auto& defines = interpreter->getDefines();
    for (const auto& [name, value] : defines)
    {
        std::string detail = value->toString();
        items.push_back(makeCompletionItem("@" + name, 6, detail, "Macro defined in [#define]"));
    }
    return items;
}

json CompletionProvider::completeSectionReferences(yini::Interpreter* interpreter, const std::string& prefix)
{
    json items = json::array();
    if (!interpreter) return items;

    const auto& sections = interpreter->getSections();

    if (prefix.find('.') == std::string::npos)
    {
        for (const auto& [section_name, section] : sections)
        {
            items.push_back(makeCompletionItem("@{" + section_name, 7, section_name, "Section"));
        }
    }
    else
    {
        size_t dot_pos = prefix.find('.');
        std::string section_name = prefix.substr(0, dot_pos);
        auto it = sections.find(section_name);
        if (it != sections.end())
        {
            const auto& section = it->second;
            for (const auto& [key, value] : section.entries)
            {
                std::string detail = value->toString();
                items.push_back(makeCompletionItem("@{" + section_name + "." + key + "}", 5, detail, "Key in [" + section_name + "]"));
            }
        }
    }
    return items;
}

json CompletionProvider::completeDataTypes(const std::string& /*prefix*/)
{
    json items = json::array();
    const std::vector<std::pair<std::string, std::string>> types = {
        {"Color", "Color type: Color(r, g, b)"},
        {"Coord", "Coordinate type: Coord(x, y) or Coord(x, y, z)"},
        {"List", "Linked list: List(item1, item2, ...)"},
        {"Path", "Path type: Path(\"file.txt\")"},
        {"Dyna", "Dynamic value: Dyna(value)"}
    };
    for (const auto& [name, desc] : types)
    {
        items.push_back(makeCompletionItem(name, 3, "Built-in type", desc));
    }
    return items;
}

json CompletionProvider::makeCompletionItem(
    const std::string& label,
    int kind,
    const std::string& detail,
    const std::string& documentation)
{
    json item = {{"label", label}, {"kind", kind}};
    if (!detail.empty())
    {
        item["detail"] = detail;
    }
    if (!documentation.empty())
    {
        item["documentation"] = documentation;
    }
    return item;
}

} // namespace yini::lsp