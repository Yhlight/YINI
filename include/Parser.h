#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include "Token.h"
#include "Lexer.h"
#include "AST.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace yini
{

// Parser class: transforms a token stream into an Abstract Syntax Tree (AST).
class Parser
{
public:
    explicit Parser(const std::vector<Token>& tokens);
    explicit Parser(const std::string& source);
    ~Parser() = default;
    
    // Main parsing function: returns the root of the AST.
    std::shared_ptr<RootNode> parse();
    
    // Error reporting
    std::string getLastError() const { return last_error; }
    bool hasError() const { return !last_error.empty(); }
    
private:
    // Token management
    Token peek() const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    
    // Parsing methods for top-level constructs
    std::shared_ptr<ASTNode> parseTopLevel();
    std::shared_ptr<SectionNode> parseSection();
    std::shared_ptr<DefineNode> parseDefineSection();
    std::shared_ptr<IncludeNode> parseIncludeSection();
    std::shared_ptr<SchemaNode> parseSchemaSection();
    
    std::shared_ptr<KeyValuePairNode> parseKeyValuePair();
    std::shared_ptr<KeyValuePairNode> parseQuickRegister();
    
    // Expression parsing (Pratt Parser)
    std::shared_ptr<ASTNode> parseExpression(int precedence = 0);
    std::shared_ptr<ASTNode> parsePrimary();
    
    // Value parsing
    std::shared_ptr<ASTNode> parseArray();
    std::shared_ptr<ASTNode> parseMap();
    std::shared_ptr<ASTNode> parseFunctionCall();
    std::shared_ptr<ASTNode> parseReference();
    std::shared_ptr<ASTNode> parseEnvVar();
    
    // Error handling
    void error(const std::string& message);
    
    // State
    std::vector<Token> tokens;
    size_t current;
    
    // Quick register counter
    int64_t quick_register_counter;
    
    // Error tracking
    std::string last_error;
};

} // namespace yini

#endif // YINI_PARSER_H
