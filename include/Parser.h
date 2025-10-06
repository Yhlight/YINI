#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include "Token.h"
#include "Value.h"
#include "Lexer.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <optional>

namespace yini
{

// Configuration section
struct Section
{
    std::string name;
    std::vector<std::string> inherited_sections;
    std::map<std::string, std::shared_ptr<Value>> entries;
    Token token; // Token for the section's name, for error reporting.
    
    Section(const std::string& name = "") : name(name)
    {
    }
};

// Schema validation rule
struct SchemaRule
{
    bool required;           // ! or ?
    std::optional<ValueType> value_type;
    enum class NullBehavior
    {
        IGNORE,
        DEFAULT,
        ERROR
    } null_behavior;
    std::shared_ptr<Value> default_value;
};

// Parser class
class Parser
{
public:
    explicit Parser(const std::vector<Token>& tokens);
    explicit Parser(const std::string& source);
    ~Parser() = default;
    
    // Main parsing function
    bool parse();
    
    // Get parsed sections
    const std::map<std::string, Section>& getSections() const { return sections; }
    
    // Get defines
    const std::map<std::string, std::shared_ptr<Value>>& getDefines() const { return defines; }
    
    // Get includes
    const std::vector<std::string>& getIncludes() const { return includes; }
    
    // Get schema
    const std::map<std::string, std::map<std::string, SchemaRule>>& getSchema() const { return schema; }
    
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
    
    // Parsing methods
    bool parseSection();
    bool parseDefineSection();
    bool parseIncludeSection();
    bool parseSchemaSection();
    
    bool parseKeyValuePair(Section& section);
    bool parseQuickRegister(Section& section);
    
    // Value parsing (Strategy pattern)
    std::shared_ptr<Value> parseValue();
    std::shared_ptr<Value> parseExpression();
    std::shared_ptr<Value> parseTerm();
    std::shared_ptr<Value> parseFactor();
    std::shared_ptr<Value> parsePrimary();
    
    std::shared_ptr<Value> parseArray();
    std::shared_ptr<Value> parseArray(Token token);
    std::shared_ptr<Value> parseList(Token token);
    std::shared_ptr<Value> parseMap(Token token);
    std::shared_ptr<Value> parseSet(Token token);
    std::shared_ptr<Value> parseColor(Token token);
    std::shared_ptr<Value> parseCoord(Token token);
    std::shared_ptr<Value> parsePath(Token token);
    std::shared_ptr<Value> parseDynamic(Token token);
    std::shared_ptr<Value> parseReference(Token token);
    std::shared_ptr<Value> parseEnvVar(Token token);
    
    // Section inheritance
    void resolveInheritance();
    
    // Schema validation
    bool validateAgainstSchema();
    
    // Reference resolution
    bool resolveReferences();
    std::shared_ptr<Value> resolveValue(
        std::shared_ptr<Value> value, 
        std::set<std::string>& visiting
    );
    
    // Error handling
    void error(const std::string& message);
    void error(const std::string& message, const Token& token);
    
    // State
    std::vector<Token> tokens;
    size_t current;
    
    // Parsed data
    std::map<std::string, Section> sections;
    std::map<std::string, std::shared_ptr<Value>> defines;
    std::vector<std::string> includes;
    std::map<std::string, std::map<std::string, SchemaRule>> schema;
    
    // Quick register counter
    int64_t quick_register_counter;
    
    // Error tracking
    std::string last_error;
};

} // namespace yini

#endif // YINI_PARSER_H
