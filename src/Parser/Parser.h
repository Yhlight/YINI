#pragma once

#include "../Lexer/Lexer.h"
#include "AST.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace YINI
{
    // Forward declaration
    class Parser;

    // --- Strategy Pattern ---
    // Abstract base class for parsing strategies within a section body
    class IParseStrategy
    {
    public:
        virtual ~IParseStrategy() = default;
        virtual std::unique_ptr<AST::Statement> parse(Parser& parser) = 0;
    };

    // --- State Machine ---
    enum class ParserState
    {
        GLOBAL,         // Looking for the start of a section
        SECTION_BODY    // Inside a section, looking for statements
    };

    class Parser
    {
    public:
        Parser(Lexer& lexer);

        std::unique_ptr<AST::Program> parseProgram();
        const std::vector<std::string>& getErrors() const;

        // Public token access for strategies
        Token& currentToken();
        Token& peekToken();
        void nextToken();

        // Public error handling for strategies
        void addError(const std::string& error);

        // Public expression parsing for strategies
        std::unique_ptr<AST::Expression> parseExpression(int precedence = 0);

    private:
        // Function types for Pratt parser
        using PrefixParseFn = std::unique_ptr<AST::Expression> (Parser::*)();
        using InfixParseFn = std::unique_ptr<AST::Expression> (Parser::*)(std::unique_ptr<AST::Expression>);

        // State-specific parsing handlers
        void parseGlobal();
        void parseSectionBody();

        // Specific parsing logic called by states/strategies
        std::unique_ptr<AST::Section> parseSectionHeader();

        // Expression parsing functions
        std::unique_ptr<AST::Expression> parseIdentifier();
        std::unique_ptr<AST::Expression> parseIntegerLiteral();
        std::unique_ptr<AST::Expression> parseFloatLiteral();
        std::unique_ptr<AST::Expression> parseStringLiteral();
        std::unique_ptr<AST::Expression> parseBooleanLiteral();
        std::unique_ptr<AST::Expression> parseMacroReference();
        std::unique_ptr<AST::Expression> parseDynaExpression();
        std::unique_ptr<AST::Expression> parseInfixExpression(std::unique_ptr<AST::Expression> left);

        // Precedence helpers
        int getTokenPrecedence(const Token& token);

        Lexer& m_lexer;
        std::vector<std::string> m_errors;

        Token m_current_token;
        Token m_peek_token;

        ParserState m_state;
        std::unique_ptr<AST::Program> m_program;
        AST::Section* m_current_section = nullptr; // Raw pointer to current section context

        // --- Pratt Parser Maps ---
        // Strategy map for statements
        std::map<TokenType, std::unique_ptr<IParseStrategy>> m_statement_strategies;
        // Map for prefix parsing functions
        std::map<TokenType, PrefixParseFn> m_prefix_parse_fns;
        // Map for infix parsing functions
        std::map<TokenType, InfixParseFn> m_infix_parse_fns;
    };
}
