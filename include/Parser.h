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
#include <mutex>

namespace yini
{

// Configuration section
struct Section
{
    std::string name;
    std::vector<std::string> inherited_sections;
    std::map<std::string, std::shared_ptr<Value>> entries;
    
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
    
    // Disable copying (expensive and unnecessary)
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    
    // Enable moving
    Parser(Parser&&) noexcept = default;
    Parser& operator=(Parser&&) noexcept = default;
    
    // Main parsing function
    bool parse();
    
    // Environment variable security
    void setSafeMode(bool enabled) { safe_mode = enabled; }
    bool isSafeModeEnabled() const { return safe_mode; }
    static void setAllowedEnvVars(const std::set<std::string>& vars);
    static void addAllowedEnvVar(const std::string& var);
    static void clearAllowedEnvVars();
    static const std::set<std::string>& getAllowedEnvVars() { return allowed_env_vars; }
    
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
    std::shared_ptr<Value> parseList();
    std::shared_ptr<Value> parseMap();
    std::shared_ptr<Value> parseTuple();
    std::shared_ptr<Value> parseSet();
    std::shared_ptr<Value> parseColor();
    std::shared_ptr<Value> parseCoord();
    std::shared_ptr<Value> parsePath();
    std::shared_ptr<Value> parseDynamic();
    
    std::shared_ptr<Value> parseReference();
    std::shared_ptr<Value> parseEnvVar();
    
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
    
    // Overflow checking helpers
    bool willOverflowAdd(int64_t a, int64_t b) const;
    bool willOverflowSubtract(int64_t a, int64_t b) const;
    bool willOverflowMultiply(int64_t a, int64_t b) const;
    
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
    
    // Recursion depth tracking
    static constexpr size_t MAX_RECURSION_DEPTH = 100;
    size_t expression_depth;
    size_t array_depth;
    
    // Resource limits
    static constexpr size_t MAX_ARRAY_SIZE = 100000;  // 100K elements
    
    // Environment variable security
    bool safe_mode;
    static std::set<std::string> allowed_env_vars;
    static std::mutex env_vars_mutex;  // Thread safety for static whitelist
    
    // Error tracking
    std::string last_error;
};

} // namespace yini

#endif // YINI_PARSER_H
