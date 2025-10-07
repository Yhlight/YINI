#ifndef YINI_PARSER_H
#define YINI_PARSER_H

#include <filesystem>
#include "Ast.h"
#include "../Lexer/Lexer.h"
namespace YINI
{

    class Parser
    {
    public:
        Parser(Lexer& lexer, const std::string& filepath = "");
        std::unique_ptr<AstNode> parse();

    private:
        void advance();
        bool match(TokenType type);
        void consume(TokenType type, const std::string& message);

        AstNode parseAst();
        void parseSpecialSection(AstNode& ast);
        SectionNode parseSection(AstNode& ast);
        KeyValueNode parseKeyValue(AstNode& ast);
        std::unique_ptr<YiniValue> parseValue(AstNode& ast);
        std::unique_ptr<YiniValue::Array> parseArray(AstNode& ast);
        std::unique_ptr<YiniValue::Map> parseMap(AstNode& ast);
        std::unique_ptr<YiniValue> parseFunctionCall(AstNode& ast, const std::string& functionName);
        std::vector<std::unique_ptr<YiniValue>> parseArgumentList(AstNode& ast);

        // Expression parsing
        std::unique_ptr<YiniValue> parseExpression(AstNode& ast);
        std::unique_ptr<YiniValue> parseTerm(AstNode& ast);
        std::unique_ptr<YiniValue> parseFactor(AstNode& ast);
        std::unique_ptr<YiniValue> parsePrimary(AstNode& ast);

        Lexer& m_lexer;
        Token m_currentToken;
        int m_plusEqualsCounter = 0;
        std::string m_filepath;
    };
}

#endif // YINI_PARSER_H