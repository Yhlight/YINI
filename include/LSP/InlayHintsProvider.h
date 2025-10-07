#ifndef YINI_INLAY_HINTS_PROVIDER_H
#define YINI_INLAY_HINTS_PROVIDER_H

#include "Parser.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class InlayHintsProvider
{
public:
    InlayHintsProvider();
    
    // Get inlay hints for range
    json getInlayHints(
        yini::Parser* parser,
        const std::string& content,
        Range range
    );
    
private:
    // Create inlay hint
    json makeInlayHint(Position position, const std::string& label, const std::string& kind);
    
    // Get hint for macro reference
    json getMacroHint(const std::string& macroName, yini::Parser* parser, Position position);
    
    // Get hint for type
    json getTypeHint(const std::string& value, Position position);
};

} // namespace yini::lsp

#endif // YINI_INLAY_HINTS_PROVIDER_H
