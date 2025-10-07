#ifndef YINI_RENAME_PROVIDER_H
#define YINI_RENAME_PROVIDER_H

#include "Interpreter.h"
#include "LSP/DocumentManager.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class RenameProvider
{
public:
    RenameProvider();
    
    // Prepare rename (check if rename is valid)
    json prepareRename(
        yini::Interpreter* interpreter,
        Document* document,
        Position position
    );
    
    // Perform rename
    json rename(
        yini::Interpreter* interpreter,
        Document* document,
        const std::string& uri,
        Position position,
        const std::string& newName
    );
    
private:
    // Get word at position
    std::string getWordAtPosition(const std::string& content, Position pos);
    
    // Get line at position
    std::string getLineAtPosition(const std::string& content, int line);
    
    // Check if position is in macro reference
    bool isMacroReference(const std::string& line, int character);
    
    // Validate identifier name
    bool isValidIdentifier(const std::string& name);
    
    // Find all occurrences and create edits
    json createMacroRenameEdits(
        const std::string& content,
        const std::string& oldName,
        const std::string& newName
    );
    
    // Create text edit
    json makeTextEdit(int line, int startChar, int endChar, const std::string& newText);
};

} // namespace yini::lsp

#endif // YINI_RENAME_PROVIDER_H
