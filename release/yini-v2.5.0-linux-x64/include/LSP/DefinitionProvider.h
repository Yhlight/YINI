#ifndef YINI_DEFINITION_PROVIDER_H
#define YINI_DEFINITION_PROVIDER_H

#include "Parser.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class DefinitionProvider
{
public:
    DefinitionProvider();
    
    // Get definition location at position
    json getDefinition(
        yini::Parser* parser,
        const std::string& content,
        const std::string& uri,
        Position position
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
    
    // Find macro definition location
    json findMacroDefinition(const std::string& content, const std::string& uri, const std::string& name);
    
    // Find section definition location
    json findSectionDefinition(const std::string& content, const std::string& uri, const std::string& section);
    
    // Find key definition location
    json findKeyDefinition(const std::string& content, const std::string& uri, const std::string& section, const std::string& key);
    
    // Create location result
    json makeLocation(const std::string& uri, int line, int character);
};

} // namespace yini::lsp

#endif // YINI_DEFINITION_PROVIDER_H
