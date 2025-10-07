#ifndef YINI_COMPLETION_PROVIDER_H
#define YINI_COMPLETION_PROVIDER_H

#include "Interpreter.h"
#include "LSP/DocumentManager.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class CompletionProvider
{
public:
    CompletionProvider();
    
    // Get completions at position
    json getCompletions(
        yini::Interpreter* interpreter,
        Document* document,
        Position position
    );
    
private:
    // Get line and character at position
    std::string getLineAtPosition(const std::string& content, int line);
    std::string getTextBeforeCursor(const std::string& content, Position pos);
    
    // Completion generators
    json completeDirectives(const std::string& prefix);
    json completeKeywords(const std::string& prefix);
    json completeMacroReferences(yini::Interpreter* interpreter, const std::string& prefix);
    json completeSectionReferences(yini::Interpreter* interpreter, const std::string& prefix);
    json completeDataTypes(const std::string& prefix);
    
    // Helper to create completion item
    json makeCompletionItem(
        const std::string& label,
        int kind,
        const std::string& detail = "",
        const std::string& documentation = ""
    );
};

} // namespace yini::lsp

#endif // YINI_COMPLETION_PROVIDER_H
