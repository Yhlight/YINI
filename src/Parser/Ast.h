#ifndef YINI_AST_H
#define YINI_AST_H

#include "../Lexer/Token.h"
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <map>

namespace Yini
{
    namespace Ast
    {
        // Base class for all nodes in the AST
        struct Node
        {
            virtual ~Node() = default;
            virtual std::string toString() const = 0;
        };

        // Base class for all expression nodes
        struct Expression : public Node {};

        // Base class for all statement nodes
        struct Statement : public Node {};

        // Represents an identifier, e.g., a key or section name
        struct Identifier : public Expression
        {
            Token token;
            std::string value;
            Identifier(Token t, std::string val) : token(std::move(t)), value(std::move(val)) {}
            std::string toString() const override { return value; }
        };

        // Represents a section, e.g., [SectionName] : Parent1, Parent2
        struct Section : public Statement
        {
            Token token; // The '[' token
            std::unique_ptr<Identifier> name;
            std::vector<std::unique_ptr<Identifier>> parents;
            std::vector<std::unique_ptr<Statement>> statements;
            std::string toString() const override
            {
                std::stringstream ss;
                ss << name->toString();
                if (!parents.empty())
                {
                    ss << " : ";
                    for (size_t i = 0; i < parents.size(); ++i)
                    {
                        ss << parents[i]->toString() << (i == parents.size() - 1 ? "" : ", ");
                    }
                }
                ss << "\n";
                for (const auto& stmt : statements)
                {
                    ss << "  " << stmt->toString() << "\n";
                }
                return ss.str();
            }
        };

        // Represents a full YINI file/document
        struct YiniDocument : public Node
        {
            std::vector<std::unique_ptr<Statement>> statements;
            std::string toString() const override
            {
                std::stringstream ss;
                for (const auto& stmt : statements)
                {
                    ss << stmt->toString();
                }
                return ss.str();
            }
        };

        // Represents a key-value pair, e.g., key = value
        struct KeyValuePair : public Statement
        {
            Token token; // The '=' token
            std::unique_ptr<Identifier> key;
            std::unique_ptr<Expression> value;
            std::string toString() const override
            {
                return key->toString() + " = " + (value ? value->toString() : "null");
            }
        };

        // Represents a quick registration, e.g., += value
        struct QuickRegister : public Statement
        {
            Token token; // The '+=' token
            std::unique_ptr<Expression> value;
            std::string toString() const override
            {
                return "+= " + (value ? value->toString() : "null");
            }
        };

        // Represents an include statement, e.g., += file.yini
        struct IncludeStatement : public Statement
        {
            Token token; // The '+=' token
            std::unique_ptr<Expression> filepath;
            std::string toString() const override
            {
                return "+= " + (filepath ? filepath->toString() : "null");
            }
        };

        // Represents a simple integer literal
        struct IntegerLiteral : public Expression
        {
            Token token;
            long long value;
            std::string toString() const override { return token.literal; }
        };

        // Represents a float literal
        struct FloatLiteral : public Expression
        {
            Token token;
            double value;
            std::string toString() const override { return token.literal; }
        };

        // Represents a boolean literal
        struct BooleanLiteral : public Expression
        {
            Token token;
            bool value;
            std::string toString() const override { return token.literal; }
        };

        // Represents a string literal
        struct StringLiteral : public Expression
        {
            Token token;
            std::string value;
            std::string toString() const override { return "\"" + value + "\""; }
        };

        // Represents an infix expression, e.g., 5 + 2
        struct InfixExpression : public Expression
        {
            Token token; // The operator token, e.g., '+'
            std::unique_ptr<Expression> left;
            std::string op;
            std::unique_ptr<Expression> right;
            std::string toString() const override
            {
                return "(" + left->toString() + " " + op + " " + right->toString() + ")";
            }
        };

        struct ColorLiteral : public Expression
        {
            Token token; // The #RRGGBB token
            std::string toString() const override { return token.literal; }
        };

        // --- NEW AST NODES ---

        struct MacroReference : public Expression
        {
            Token token; // The '@' token
            std::unique_ptr<Identifier> name;
            std::string toString() const override { return "@" + name->toString(); }
        };

        struct ArrayLiteral : public Expression
        {
            Token token; // The '[' token
            std::vector<std::unique_ptr<Expression>> elements;
            std::string toString() const override { /* TODO */ return "[]"; }
        };

        struct KeyValueLiteral : public Expression
        {
            Token token; // The '{' token
            std::unique_ptr<Identifier> key;
            std::unique_ptr<Expression> value;
            std::string toString() const override { return "{}"; }
        };

        struct MapLiteral : public Expression
        {
            Token token; // The '{' token
            std::vector<std::unique_ptr<KeyValueLiteral>> elements;
            std::string toString() const override { return "{{}}"; }
        };

        struct FunctionCall : public Expression
        {
            Token token; // The function name token (e.g., 'Coord')
            std::unique_ptr<Identifier> functionName;
            std::vector<std::unique_ptr<Expression>> arguments;
            std::string toString() const override { return functionName->toString() + "(...)"; }
        };

        // Dyna, Coord, Color, Path can all be represented as FunctionCall nodes
        // This simplifies the AST and moves the logic to the runtime.

    } // namespace Ast
} // namespace Yini

#endif // YINI_AST_H
