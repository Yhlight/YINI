#ifndef YINI_FORMATTING_PROVIDER_H
#define YINI_FORMATTING_PROVIDER_H

#include "LSP/LSPTypes.h"
#include <nlohmann/json.hpp>
#include <string>

namespace yini::lsp
{

using json = nlohmann::json;

struct FormattingOptions
{
    int tabSize;
    bool insertSpaces;
    bool trimTrailingWhitespace;
    bool insertFinalNewline;
};

class FormattingProvider
{
public:
    FormattingProvider();
    
    // Format entire document
    json formatDocument(
        const std::string& content,
        const FormattingOptions& options
    );
    
    // Format range
    json formatRange(
        const std::string& content,
        Range range,
        const FormattingOptions& options
    );
    
private:
    // Format a single line
    std::string formatLine(const std::string& line, const FormattingOptions& options);
    
    // Detect indentation level
    int getIndentLevel(const std::string& line);
    
    // Check if line is section header
    bool isSectionHeader(const std::string& line);
    
    // Check if line is key-value pair
    bool isKeyValuePair(const std::string& line);
    
    // Trim trailing whitespace
    std::string trimTrailing(const std::string& line);
    
    // Create text edit
    json makeTextEdit(int line, int startChar, int endChar, const std::string& newText);
};

} // namespace yini::lsp

#endif // YINI_FORMATTING_PROVIDER_H
