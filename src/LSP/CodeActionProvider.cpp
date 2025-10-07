#include "LSP/CodeActionProvider.h"
#include <sstream>

namespace yini::lsp
{

CodeActionProvider::CodeActionProvider()
{
}

json CodeActionProvider::makeTextEdit(Range range, const std::string& newText)
{
    return {
        {"range", {
            {"start", {{"line", range.start.line}, {"character", range.start.character}}},
            {"end", {{"line", range.end.line}, {"character", range.end.character}}}
        }},
        {"newText", newText}
    };
}

json CodeActionProvider::createQuickFix(const std::string& title, const std::string& uri, const json& edits)
{
    return {
        {"title", title},
        {"kind", "quickfix"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::createExtractToMacro(const std::string& uri, Range range, const std::string& /*content*/)
{
    // Extract selected value to a macro
    json edits = json::array();
    
    // TODO: Implement actual extraction logic
    return {
        {"title", "Extract to macro"},
        {"kind", "refactor.extract"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::createConvertToReference(const std::string& uri, Range range, const std::string& /*content*/)
{
    json edits = json::array();
    
    // TODO: Convert hardcoded value to @{Section.key} reference
    return {
        {"title", "Convert to reference"},
        {"kind", "refactor.rewrite"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::createInlineValue(const std::string& uri, Range range, const std::string& /*content*/)
{
    json edits = json::array();
    
    // TODO: Inline @macro to its actual value
    return {
        {"title", "Inline value"},
        {"kind", "refactor.inline"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::createOrganizeImports(const std::string& uri, const std::string& content)
{
    (void)content;
    json edits = json::array();
    
    // TODO: Sort [#include] statements
    return {
        {"title", "Organize includes"},
        {"kind", "source.organizeImports"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::createSortKeys(const std::string& uri, const std::string& content)
{
    (void)content;
    json edits = json::array();
    
    // TODO: Sort keys in sections alphabetically
    return {
        {"title", "Sort keys alphabetically"},
        {"kind", "source"},
        {"edit", {
            {"changes", {
                {uri, edits}
            }}
        }}
    };
}

json CodeActionProvider::getCodeActions(
    yini::Parser* /*parser*/,
    const std::string& content,
    const std::string& uri,
    Range range,
    const json& context)
{
    (void)context;
    json actions = json::array();
    
    // Always offer source actions
    actions.push_back(createOrganizeImports(uri, content));
    actions.push_back(createSortKeys(uri, content));
    
    // Offer refactoring actions based on selection
    if (range.start.line != range.end.line || range.start.character != range.end.character)
    {
        actions.push_back(createExtractToMacro(uri, range, content));
        actions.push_back(createConvertToReference(uri, range, content));
    }
    
    return actions;
}

} // namespace yini::lsp
