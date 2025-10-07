#ifndef YINI_SEMANTIC_TOKENS_PROVIDER_H
#define YINI_SEMANTIC_TOKENS_PROVIDER_H

#include "Interpreter.h"
#include "LSP/DocumentManager.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace yini::lsp
{

using json = nlohmann::json;

// LSP Semantic Token Types
enum class SemanticTokenType
{
    NAMESPACE = 0,    // [#define], [#include]
    CLASS = 1,        // [Section]
    ENUM = 2,
    INTERFACE = 3,
    STRUCT = 4,
    TYPE_PARAMETER = 5,
    PARAMETER = 6,
    VARIABLE = 7,     // key names
    PROPERTY = 8,     // key in section
    ENUM_MEMBER = 9,
    DECORATOR = 10,
    EVENT = 11,
    FUNCTION = 12,
    METHOD = 13,
    MACRO = 14,       // @macro references
    LABEL = 15,
    COMMENT = 16,
    STRING = 17,
    KEYWORD = 18,
    NUMBER = 19,
    REGEXP = 20,
    OPERATOR = 21
};

// LSP Semantic Token Modifiers
enum class SemanticTokenModifier
{
    DECLARATION = 0,
    DEFINITION = 1,
    READONLY = 2,
    STATIC = 3,
    DEPRECATED = 4,
    ABSTRACT = 5,
    ASYNC = 6,
    MODIFICATION = 7,
    DOCUMENTATION = 8,
    DEFAULT_LIBRARY = 9
};

struct SemanticToken
{
    int line;
    int startChar;
    int length;
    SemanticTokenType type;
    int modifiers;
};

class SemanticTokensProvider
{
public:
    SemanticTokensProvider();
    
    // Get semantic tokens legend (types and modifiers)
    json getLegend();
    
    // Get semantic tokens for entire document
    json getSemanticTokens(
        yini::Interpreter* interpreter,
        Document* document
    );
    
    // Get semantic tokens for range
    json getSemanticTokensRange(
        yini::Interpreter* interpreter,
        Document* document,
        int startLine,
        int endLine
    );
    
private:
    std::vector<SemanticToken> tokens;
    
    // Extract tokens from parsed content
    void extractTokens(yini::Interpreter* interpreter, Document* document);
    
    // Add token
    void addToken(int line, int startChar, int length, SemanticTokenType type, int modifiers = 0);
    
    // Convert tokens to LSP format (delta-encoded)
    json encodeTokens();
    
    // Get line at position
    std::string getLineAtPosition(const std::string& content, int line);
    
    // Find pattern in line
    void findAndAddTokens(const std::string& line, int lineNum, const std::string& pattern, SemanticTokenType type);
};

} // namespace yini::lsp

#endif // YINI_SEMANTIC_TOKENS_PROVIDER_H
