#include "Parser.h"
#include <stdexcept>

namespace yini
{

// Helper to get precedence for infix operators
int getPrecedence(TokenType type)
{
    switch (type)
    {
        case TokenType::PLUS:
        case TokenType::MINUS:
            return 1;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO:
            return 2;
        default:
            return 0; // Not an infix operator
    }
}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0), quick_register_counter(0), last_error("") {}

Parser::Parser(const std::string& source)
    : current(0), quick_register_counter(0), last_error("")
{
    Lexer lexer(source);
    tokens = lexer.tokenize();
    if (lexer.hasError())
    {
        last_error = lexer.getLastError();
    }
}

std::shared_ptr<RootNode> Parser::parse()
{
    if (hasError())
    {
        return nullptr;
    }

    auto root = std::make_shared<RootNode>();
    while (!isAtEnd())
    {
        if (match(TokenType::NEWLINE))
        {
            continue;
        }
        auto node = parseTopLevel();
        if (!node)
        {
            return nullptr; // Error occurred
        }
        root->children.push_back(node);
    }
    return root;
}

std::shared_ptr<ASTNode> Parser::parseTopLevel()
{
    if (!check(TokenType::LBRACKET))
    {
        error("Expected a section header `[` at the top level");
        return nullptr;
    }

    // Look ahead to see if it's a special section like [#define]
    if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::HASH)
    {
        advance(); // [
        advance(); // #

        Token directive = advance();
        if (directive.type != TokenType::IDENTIFIER)
        {
            error("Expected directive name after `[#`");
            return nullptr;
        }

        std::string dir_name = directive.getValue<std::string>();
        if (dir_name == "define") return parseDefineSection();
        if (dir_name == "include") return parseIncludeSection();
        if (dir_name == "schema") return parseSchemaSection();

        error("Unknown directive: " + dir_name);
        return nullptr;
    }

    return parseSection();
}

std::shared_ptr<SectionNode> Parser::parseSection()
{
    auto node = std::make_shared<SectionNode>();

    if (!match(TokenType::LBRACKET))
    {
        error("Expected `[` to start a section");
        return nullptr;
    }

    Token name_token = advance();
    if (name_token.type != TokenType::IDENTIFIER)
    {
        error("Expected section name");
        return nullptr;
    }
    node->name = name_token.getValue<std::string>();

    if (!match(TokenType::RBRACKET))
    {
        error("Expected `]` after section name");
        return nullptr;
    }

    // Parse inheritance
    if (match(TokenType::COLON))
    {
        do
        {
            Token inherited = advance();
            if (inherited.type != TokenType::IDENTIFIER)
            {
                error("Expected section name in inheritance list");
                return nullptr;
            }
            node->inherited_sections.push_back(inherited.getValue<std::string>());
        } while (match(TokenType::COMMA));
    }

    while (match(TokenType::NEWLINE)) {}

    // Parse key-value pairs
    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        if (match(TokenType::NEWLINE)) continue;

        std::shared_ptr<KeyValuePairNode> kvp;
        if (check(TokenType::PLUS_EQUALS))
        {
            kvp = parseQuickRegister();
        }
        else if (check(TokenType::IDENTIFIER))
        {
            kvp = parseKeyValuePair();
        }
        else
        {
            break; // End of section
        }

        if (!kvp) return nullptr; // Propagate error
        node->children.push_back(kvp);
    }

    return node;
}

std::shared_ptr<DefineNode> Parser::parseDefineSection()
{
    auto node = std::make_shared<DefineNode>();
    if (!match(TokenType::RBRACKET))
    {
        error("Expected `]` after [#define");
        return nullptr;
    }

    while (match(TokenType::NEWLINE)) {}

    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        if (match(TokenType::NEWLINE)) continue;
        auto kvp = parseKeyValuePair();
        if (!kvp) return nullptr;
        node->definitions.push_back(kvp);
    }
    return node;
}

std::shared_ptr<IncludeNode> Parser::parseIncludeSection()
{
    auto node = std::make_shared<IncludeNode>();
    if (!match(TokenType::RBRACKET))
    {
        error("Expected `]` after [#include");
        return nullptr;
    }

    while (match(TokenType::NEWLINE)) {}

    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        if (match(TokenType::NEWLINE)) continue;
        if (!match(TokenType::PLUS_EQUALS))
        {
            error("Expected `+=` in include section");
            return nullptr;
        }
        Token path = advance();
        if (path.type != TokenType::STRING && path.type != TokenType::IDENTIFIER)
        {
            error("Expected path in include section");
            return nullptr;
        }
        node->files.push_back(path.getValue<std::string>());
    }
    return node;
}

std::shared_ptr<SchemaNode> Parser::parseSchemaSection()
{
    // For now, just consume the schema section without parsing its content,
    // as the interpreter logic is out of scope for this refactoring.
    if (!match(TokenType::RBRACKET))
    {
        error("Expected `]` after [#schema");
        return nullptr;
    }
    while (!isAtEnd() && !check(TokenType::LBRACKET))
    {
        advance();
    }
    return std::make_shared<SchemaNode>();
}

std::shared_ptr<KeyValuePairNode> Parser::parseKeyValuePair()
{
    auto node = std::make_shared<KeyValuePairNode>();
    Token key = advance();
    if (key.type != TokenType::IDENTIFIER)
    {
        error("Expected key identifier");
        return nullptr;
    }
    node->key = key.getValue<std::string>();

    if (!match(TokenType::EQUALS))
    {
        error("Expected `=` after key");
        return nullptr;
    }

    node->value = parseExpression();
    if (!node->value) return nullptr;

    return node;
}

std::shared_ptr<KeyValuePairNode> Parser::parseQuickRegister()
{
    auto node = std::make_shared<KeyValuePairNode>();
    if (!match(TokenType::PLUS_EQUALS))
    {
        error("Expected `+=`");
        return nullptr;
    }
    node->key = std::to_string(quick_register_counter++);
    node->value = parseExpression();
    if (!node->value) return nullptr;

    return node;
}

std::shared_ptr<ASTNode> Parser::parseExpression(int precedence)
{
    auto left = parsePrimary();
    if (!left) return nullptr;

    while (precedence < getPrecedence(peek().type))
    {
        Token op = advance();
        auto right = parseExpression(getPrecedence(op.type));
        if (!right) return nullptr;
        
        auto new_left = std::make_shared<BinaryOpNode>();
        new_left->left = left;
        new_left->op = op;
        new_left->right = right;
        left = new_left;
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parsePrimary()
{
    // Unary minus
    if (match(TokenType::MINUS))
    {
        auto node = std::make_shared<UnaryOpNode>();
        node->op = tokens[current - 1];
        node->right = parsePrimary();
        if (!node->right) return nullptr;
        return node;
    }

    // Grouping
    if (match(TokenType::LPAREN))
    {
        auto expr = parseExpression();
        if (!match(TokenType::RPAREN))
        {
            error("Expected `)` after expression");
            return nullptr;
        }
        return expr;
    }

    // Literals
    if (check(TokenType::INTEGER) || check(TokenType::FLOAT) || check(TokenType::BOOLEAN) || check(TokenType::STRING))
    {
        Token t = advance();
        auto literal = std::make_shared<LiteralNode>();
        if (t.type == TokenType::INTEGER) literal->value = std::make_shared<Value>(t.getValue<int64_t>());
        else if (t.type == TokenType::FLOAT) literal->value = std::make_shared<Value>(t.getValue<double>());
        else if (t.type == TokenType::BOOLEAN) literal->value = std::make_shared<Value>(t.getValue<bool>());
        else if (t.type == TokenType::STRING) literal->value = std::make_shared<Value>(t.getValue<std::string>());
        return literal;
    }
    
    // Hex Color Literal or Function Call
    if (peek().type == TokenType::COLOR)
    {
        Token t = peek();
        // Check if it's a hex string like #FF0000
        if (t.hasValue())
        {
            const auto& val = t.getValue<std::string>();
            if (!val.empty() && val[0] == '#')
            {
                advance(); // consume hex color token
                auto literal = std::make_shared<LiteralNode>();
                if (val.length() == 7 || val.length() == 9)
                {
                    uint8_t r = std::stoi(val.substr(1, 2), nullptr, 16);
                    uint8_t g = std::stoi(val.substr(3, 2), nullptr, 16);
                    uint8_t b = std::stoi(val.substr(5, 2), nullptr, 16);
                    std::optional<uint8_t> a = (val.length() == 9) ? std::optional<uint8_t>(std::stoi(val.substr(7, 2), nullptr, 16)) : std::nullopt;
                    literal->value = std::make_shared<Value>(Color(r, g, b, a));
                    return literal;
                }
            }
        }
        // Otherwise, it must be a function call like Color(...)
        return parseFunctionCall();
    }

    // Function-style calls (Coord, Path, Dyna, List)
    if (check(TokenType::COORD) || check(TokenType::PATH) || check(TokenType::DYNA) || check(TokenType::LIST))
    {
        return parseFunctionCall();
    }

    // Array
    if (check(TokenType::LBRACKET)) return parseArray();

    // Map
    if (check(TokenType::LBRACE)) return parseMap();

    // References
    if (check(TokenType::AT) || check(TokenType::AT_LBRACE)) return parseReference();

    // Environment variables
    if (check(TokenType::DOLLAR_LBRACE)) return parseEnvVar();

    error("Unexpected token in expression: " + peek().toString());
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseArray()
{
    auto node = std::make_shared<ArrayNode>();
    if (!match(TokenType::LBRACKET))
    {
        error("Expected `[` at start of array");
        return nullptr;
    }
    if (!check(TokenType::RBRACKET))
    {
        do
        {
            auto element = parseExpression();
            if (!element) return nullptr;
            node->elements.push_back(element);
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RBRACKET))
    {
        error("Expected `]` at end of array");
        return nullptr;
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseMap()
{
    auto node = std::make_shared<MapNode>();
    if (!match(TokenType::LBRACE))
    {
        error("Expected `{` at start of map");
        return nullptr;
    }
    if (!check(TokenType::RBRACE))
    {
        do
        {
            Token key_token = advance();
            if (key_token.type != TokenType::IDENTIFIER && key_token.type != TokenType::STRING)
            {
                error("Expected key in map");
                return nullptr;
            }
            if (!match(TokenType::COLON))
            {
                error("Expected `:` after map key");
                return nullptr;
            }
            auto value_node = parseExpression();
            if (!value_node) return nullptr;

            auto kvp = std::make_shared<KeyValuePairNode>();
            kvp->key = key_token.getValue<std::string>();
            kvp->value = value_node;
            node->pairs.push_back(kvp);
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RBRACE))
    {
        error("Expected `}` at end of map");
        return nullptr;
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseFunctionCall()
{
    auto node = std::make_shared<FunctionCallNode>();
    node->callee_name = advance().getValue<std::string>(); // Consume keyword

    if (!match(TokenType::LPAREN))
    {
        error("Expected `(` after function name");
        return nullptr;
    }

    if (!check(TokenType::RPAREN))
    {
        do
        {
            auto arg = parseExpression();
            if (!arg) return nullptr;
            node->arguments.push_back(arg);
        } while (match(TokenType::COMMA));
    }

    if (!match(TokenType::RPAREN))
    {
        error("Expected `)` at end of argument list");
        return nullptr;
    }

    // Special case for Dyna, which is a keyword not a value type
    if (node->callee_name == "Dyna" || node->callee_name == "dyna")
    {
        auto dyna_node = std::make_shared<DynamicNode>();
        if (node->arguments.size() != 1) {
            error("Dyna() expects exactly one argument");
            return nullptr;
        }
        dyna_node->value = node->arguments[0];
        return dyna_node;
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseReference()
{
    auto node = std::make_shared<ReferenceNode>();
    if (match(TokenType::AT))
    {
        Token name = advance();
        if (name.type != TokenType::IDENTIFIER)
        {
            error("Expected identifier after `@`");
            return nullptr;
        }
        node->name = name.getValue<std::string>();
    }
    else if (match(TokenType::AT_LBRACE))
    {
        std::string ref_str;
        Token ident = advance();
        if (ident.type != TokenType::IDENTIFIER)
        {
            error("Expected identifier in cross-section reference");
            return nullptr;
        }
        ref_str = ident.getValue<std::string>();
        while (match(TokenType::DOT))
        {
            Token next = advance();
            if (next.type != TokenType::IDENTIFIER)
            {
                error("Expected identifier after `.` in reference");
                return nullptr;
            }
            ref_str += "." + next.getValue<std::string>();
        }
        if (!match(TokenType::RBRACE))
        {
            error("Expected `}` at end of reference");
            return nullptr;
        }
        node->name = ref_str;
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseEnvVar()
{
    auto node = std::make_shared<EnvVarNode>();
    if (!match(TokenType::DOLLAR_LBRACE))
    {
        error("Expected `${`");
        return nullptr;
    }
    Token name = advance();
    if (name.type != TokenType::IDENTIFIER)
    {
        error("Expected identifier in environment variable");
        return nullptr;
    }
    node->name = name.getValue<std::string>();
    if (!match(TokenType::RBRACE))
    {
        error("Expected `}` at end of environment variable");
        return nullptr;
    }
    return node;
}

// Token management
Token Parser::peek() const
{
    if (isAtEnd()) return Token(TokenType::END_OF_FILE);
    return tokens[current];
}

Token Parser::advance()
{
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::isAtEnd() const
{
    return current >= tokens.size() || tokens[current].type == TokenType::END_OF_FILE;
}

void Parser::error(const std::string& message)
{
    if (hasError()) return; // Report only the first error
    Token t = peek();
    last_error = "Parse error at line " + std::to_string(t.line) + ", column " + std::to_string(t.column) + ": " + message;
}

} // namespace yini