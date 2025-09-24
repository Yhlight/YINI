#include "Parser.h"
#include <memory>
#include <stdexcept>

namespace YINI
{
    // --- Precedence levels for operators ---
    enum Precedence
    {
        LOWEST,
        SUM,         // + or -
        PRODUCT,     // * or / or %
        // Add more as needed, e.g., PREFIX for -x or !x
    };

    static const std::map<TokenType, Precedence> precedences = {
        {TokenType::PLUS, SUM},
        {TokenType::MINUS, SUM},
        {TokenType::STAR, PRODUCT},
        {TokenType::SLASH, PRODUCT},
        {TokenType::PERCENT, PRODUCT},
    };

    // --- Concrete Strategy for parsing "key = value" ---
    class KeyValuePairParseStrategy : public IParseStrategy
    {
    public:
        std::unique_ptr<AST::Statement> parse(Parser& parser) override;
    };

    std::unique_ptr<AST::Statement> KeyValuePairParseStrategy::parse(Parser& parser)
    {
        auto kvp = std::make_unique<AST::KeyValuePair>();
        auto key_ident = std::make_unique<AST::Identifier>();
        key_ident->token = parser.currentToken();
        key_ident->value = parser.currentToken().literal;
        kvp->key = std::move(key_ident);

        if (parser.peekToken().type != TokenType::EQUAL)
        {
            parser.addError("Expected '=' after key '" + kvp->key->value + "'");
            return nullptr;
        }
        parser.nextToken();
        parser.nextToken();

        kvp->value = parser.parseExpression();
        return kvp;
    }


    // --- Parser Implementation ---
    Parser::Parser(Lexer& lexer) : m_lexer(lexer), m_state(ParserState::GLOBAL)
    {
        // Setup strategies for statements
        m_statement_strategies[TokenType::IDENTIFIER] = std::make_unique<KeyValuePairParseStrategy>();

        // Setup prefix parsing functions
        m_prefix_parse_fns[TokenType::IDENTIFIER] = &Parser::parseIdentifier;
        m_prefix_parse_fns[TokenType::INTEGER] = &Parser::parseIntegerLiteral;
        m_prefix_parse_fns[TokenType::FLOAT] = &Parser::parseFloatLiteral;
        m_prefix_parse_fns[TokenType::STRING] = &Parser::parseStringLiteral;
        m_prefix_parse_fns[TokenType::KEYWORD_TRUE] = &Parser::parseBooleanLiteral;
        m_prefix_parse_fns[TokenType::KEYWORD_FALSE] = &Parser::parseBooleanLiteral;
        m_prefix_parse_fns[TokenType::AT] = &Parser::parseMacroReference;
        m_prefix_parse_fns[TokenType::KEYWORD_DYNA] = &Parser::parseDynaExpression;

        // Setup infix parsing functions
        m_infix_parse_fns[TokenType::PLUS] = &Parser::parseInfixExpression;
        m_infix_parse_fns[TokenType::MINUS] = &Parser::parseInfixExpression;
        m_infix_parse_fns[TokenType::STAR] = &Parser::parseInfixExpression;
        m_infix_parse_fns[TokenType::SLASH] = &Parser::parseInfixExpression;
        m_infix_parse_fns[TokenType::PERCENT] = &Parser::parseInfixExpression;

        nextToken();
        nextToken();
    }

    // ... (utility methods like nextToken, getErrors, etc. remain the same) ...
    void Parser::nextToken() { m_current_token = m_peek_token; m_peek_token = m_lexer.nextToken(); }
    Token& Parser::currentToken() { return m_current_token; }
    Token& Parser::peekToken() { return m_peek_token; }
    void Parser::addError(const std::string& error) { m_errors.push_back(error); }
    const std::vector<std::string>& Parser::getErrors() const { return m_errors; }

    // ... (state machine methods parseProgram, parseGlobal, parseSectionBody remain the same) ...
    std::unique_ptr<AST::Program> Parser::parseProgram() {
        m_program = std::make_unique<AST::Program>();
        while (m_current_token.type != TokenType::END_OF_FILE) {
            switch (m_state) {
                case ParserState::GLOBAL: parseGlobal(); break;
                case ParserState::SECTION_BODY: parseSectionBody(); break;
            }
            nextToken();
        }
        return std::move(m_program);
    }
    void Parser::parseGlobal() {
        if (m_current_token.type == TokenType::L_BRACKET) {
            auto section = parseSectionHeader();
            if (section) {
                m_current_section = section.get();
                m_program->statements.push_back(std::move(section));
                m_state = ParserState::SECTION_BODY;
            }
        }
    }
    void Parser::parseSectionBody() {
        if (m_current_token.type == TokenType::L_BRACKET) {
            m_state = ParserState::GLOBAL;
            parseGlobal();
            return;
        }
        auto it = m_statement_strategies.find(m_current_token.type);
        if (it != m_statement_strategies.end()) {
            auto stmt = it->second->parse(*this);
            if (stmt && m_current_section) {
                m_current_section->statements.push_back(std::move(stmt));
            }
        }
    }
    std::unique_ptr<AST::Section> Parser::parseSectionHeader() {
        auto section = std::make_unique<AST::Section>();
        if (m_peek_token.type != TokenType::IDENTIFIER && m_peek_token.type != TokenType::HASH) {
            addError("Expected section name after '['"); return nullptr;
        }
        nextToken();
        std::string section_name_literal;
        if (m_current_token.type == TokenType::HASH) {
            if (m_peek_token.type != TokenType::IDENTIFIER) {
                addError("Expected identifier after '#'"); return nullptr;
            }
            section_name_literal = "#" + m_peek_token.literal;
            nextToken();
        } else {
            section_name_literal = m_current_token.literal;
        }
        auto name = std::make_unique<AST::Identifier>();
        name->token = m_current_token;
        name->value = section_name_literal;
        section->name = std::move(name);
        if (m_peek_token.type != TokenType::R_BRACKET) {
            addError("Expected ']' after section name"); return nullptr;
        }
        nextToken();
        return section;
    }

    // --- Expression Parsing (Pratt Parser) ---
    int Parser::getTokenPrecedence(const Token& token)
    {
        if (precedences.count(token.type))
        {
            return precedences.at(token.type);
        }
        return LOWEST;
    }

    std::unique_ptr<AST::Expression> Parser::parseExpression(int precedence)
    {
        auto prefix_it = m_prefix_parse_fns.find(m_current_token.type);
        if (prefix_it == m_prefix_parse_fns.end())
        {
            addError("No prefix parse function for token " + m_current_token.literal);
            return nullptr;
        }
        auto left_exp = (this->*(prefix_it->second))();

        while (precedence < getTokenPrecedence(m_peek_token))
        {
            auto infix_it = m_infix_parse_fns.find(m_peek_token.type);
            if (infix_it == m_infix_parse_fns.end())
            {
                return left_exp;
            }
            nextToken();
            left_exp = (this->*(infix_it->second))(std::move(left_exp));
        }

        return left_exp;
    }

    std::unique_ptr<AST::Expression> Parser::parseInfixExpression(std::unique_ptr<AST::Expression> left)
    {
        auto expression = std::make_unique<AST::InfixExpression>();
        expression->left = std::move(left);
        expression->token = m_current_token;

        int precedence = getTokenPrecedence(m_current_token);
        nextToken();
        expression->right = parseExpression(precedence);

        return expression;
    }

    // ... (literal and identifier parsing functions remain the same) ...
    std::unique_ptr<AST::Expression> Parser::parseIdentifier() { auto ident = std::make_unique<AST::Identifier>(); ident->token = m_current_token; ident->value = m_current_token.literal; return ident; }
    std::unique_ptr<AST::Expression> Parser::parseIntegerLiteral() { auto literal = std::make_unique<AST::IntegerLiteral>(); literal->token = m_current_token; try { literal->value = std::stoll(m_current_token.literal); } catch (const std::invalid_argument& ia) { addError("Could not parse as integer"); return nullptr; } return literal; }
    std::unique_ptr<AST::Expression> Parser::parseFloatLiteral() { auto literal = std::make_unique<AST::FloatLiteral>(); literal->token = m_current_token; try { literal->value = std::stod(m_current_token.literal); } catch (const std::invalid_argument& ia) { addError("Could not parse as float"); return nullptr; } return literal; }
    std::unique_ptr<AST::Expression> Parser::parseStringLiteral() { auto literal = std::make_unique<AST::StringLiteral>(); literal->token = m_current_token; literal->value = m_current_token.literal; return literal; }
    std::unique_ptr<AST::Expression> Parser::parseBooleanLiteral() { auto literal = std::make_unique<AST::BooleanLiteral>(); literal->token = m_current_token; literal->value = (m_current_token.type == TokenType::KEYWORD_TRUE); return literal; }
    std::unique_ptr<AST::Expression> Parser::parseMacroReference() { auto macro_ref = std::make_unique<AST::MacroReference>(); macro_ref->token = m_current_token; if (m_peek_token.type != TokenType::IDENTIFIER) { addError("Expected identifier after '@'"); return nullptr; } nextToken(); auto ident = std::make_unique<AST::Identifier>(); ident->token = m_current_token; ident->value = m_current_token.literal; macro_ref->name = std::move(ident); return macro_ref; }

    std::unique_ptr<AST::Expression> Parser::parseDynaExpression()
    {
        auto dyna_expr = std::make_unique<AST::DynaExpression>();
        dyna_expr->token = m_current_token;

        if (m_peek_token.type != TokenType::L_PAREN)
        {
            addError("Expected '(' after 'Dyna'");
            return nullptr;
        }
        nextToken(); // Consume 'Dyna'
        nextToken(); // Consume '('

        dyna_expr->wrapped_expression = parseExpression();

        if (m_peek_token.type != TokenType::R_PAREN)
        {
            addError("Expected ')' after expression in Dyna()");
            return nullptr;
        }
        nextToken(); // Consume expression

        return dyna_expr;
    }
}
