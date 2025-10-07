#ifndef YINI_SYMBOL_PROVIDER_H
#define YINI_SYMBOL_PROVIDER_H

#include "Interpreter.h"
#include "LSP/DocumentManager.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace yini::lsp
{

using json = nlohmann::json;

class SymbolProvider
{
public:
    SymbolProvider();
    
    // Get document symbols
    json getDocumentSymbols(
        yini::Interpreter* interpreter,
        Document* document
    );
    
private:
    // Symbol kinds (LSP spec)
    static constexpr int SYMBOL_FILE = 1;
    static constexpr int SYMBOL_MODULE = 2;
    static constexpr int SYMBOL_NAMESPACE = 3;
    static constexpr int SYMBOL_CLASS = 5;
    static constexpr int SYMBOL_PROPERTY = 7;
    static constexpr int SYMBOL_FIELD = 8;
    static constexpr int SYMBOL_VARIABLE = 13;
    
    // Find section position in content
    Position findSectionPosition(const std::string& content, const std::string& section);
    
    // Find key position in section
    Position findKeyPosition(const std::string& content, const std::string& section, const std::string& key);
    
    // Create document symbol
    json makeSymbol(
        const std::string& name,
        int kind,
        Range range,
        Range selectionRange,
        const json& children = json::array()
    );
};

} // namespace yini::lsp

#endif // YINI_SYMBOL_PROVIDER_H
