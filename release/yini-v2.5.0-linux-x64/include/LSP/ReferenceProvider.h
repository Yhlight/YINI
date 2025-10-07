#ifndef YINI_REFERENCE_PROVIDER_H
#define YINI_REFERENCE_PROVIDER_H

#include "Parser.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace yini::lsp
{

using json = nlohmann::json;

class ReferenceProvider
{
public:
    ReferenceProvider();
    
    // Find all references to symbol at position
    json findReferences(
        yini::Parser* parser,
        const std::string& content,
        const std::string& uri,
        Position position,
        bool includeDeclaration
    );
    
private:
    // Get word at position
    std::string getWordAtPosition(const std::string& content, Position pos);
    
    // Get line at position
    std::string getLineAtPosition(const std::string& content, int line);
    
    // Check if position is in macro reference
    bool isMacroReference(const std::string& line, int character);
    
    // Check if position is in cross-section reference
    bool isCrossSectionReference(const std::string& line, int character);
    
    // Find all macro references
    json findMacroReferences(
        const std::string& content,
        const std::string& uri,
        const std::string& macroName,
        bool includeDeclaration
    );
    
    // Find all key references (cross-section references)
    json findKeyReferences(
        const std::string& content,
        const std::string& uri,
        const std::string& section,
        const std::string& key,
        bool includeDeclaration
    );
    
    // Create location result
    json makeLocation(const std::string& uri, int line, int startChar, int endChar);
};

} // namespace yini::lsp

#endif // YINI_REFERENCE_PROVIDER_H
