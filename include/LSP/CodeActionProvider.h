#ifndef YINI_CODE_ACTION_PROVIDER_H
#define YINI_CODE_ACTION_PROVIDER_H

#include "Parser.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace yini::lsp
{

using json = nlohmann::json;

enum class CodeActionKind
{
    QUICK_FIX,
    REFACTOR,
    REFACTOR_EXTRACT,
    REFACTOR_INLINE,
    REFACTOR_REWRITE,
    SOURCE,
    SOURCE_ORGANIZE_IMPORTS
};

class CodeActionProvider
{
public:
    CodeActionProvider();
    
    // Get code actions for diagnostics
    json getCodeActions(
        yini::Parser* parser,
        const std::string& content,
        const std::string& uri,
        Range range,
        const json& context
    );
    
private:
    // Quick fixes
    json createQuickFix(const std::string& title, const std::string& uri, const json& edits);
    
    // Refactor actions
    json createExtractToMacro(const std::string& uri, Range range, const std::string& content);
    json createConvertToReference(const std::string& uri, Range range, const std::string& content);
    json createInlineValue(const std::string& uri, Range range, const std::string& content);
    
    // Source actions
    json createOrganizeImports(const std::string& uri, const std::string& content);
    json createSortKeys(const std::string& uri, const std::string& content);
    
    // Helper to create text edit
    json makeTextEdit(Range range, const std::string& newText);
};

} // namespace yini::lsp

#endif // YINI_CODE_ACTION_PROVIDER_H
