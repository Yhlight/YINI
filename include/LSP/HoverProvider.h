#ifndef YINI_HOVER_PROVIDER_H
#define YINI_HOVER_PROVIDER_H

#include "Interpreter.h"
#include "LSP/DocumentManager.h"
#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

class HoverProvider
{
public:
    HoverProvider();
    
    // Get hover information at position
    json getHover(
        yini::Interpreter* interpreter,
        Document* document,
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
    
    // Get macro hover info
    json getMacroHover(yini::Interpreter* interpreter, const std::string& name);
    
    // Get section key hover info
    json getSectionKeyHover(yini::Interpreter* interpreter, const std::string& section, const std::string& key);
    
    // Get value type string
    std::string getValueTypeString(std::shared_ptr<yini::Value> value);
    
    // Format hover content
    json makeHoverContent(const std::string& content, const std::string& language = "");
};

} // namespace yini::lsp

#endif // YINI_HOVER_PROVIDER_H
