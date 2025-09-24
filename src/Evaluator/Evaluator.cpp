#include "Evaluator.h"

namespace YINI
{
    std::unique_ptr<YiniObject> Evaluator::evaluate(AST::Node* node)
    {
        if (auto* integer_literal = dynamic_cast<AST::IntegerLiteral*>(node))
        {
            auto obj = std::make_unique<YiniObject>();
            obj->type = YiniObject::Type::INTEGER;
            obj->value = integer_literal->value;
            return obj;
        }
        else if (auto* infix_expr = dynamic_cast<AST::InfixExpression*>(node))
        {
            return evaluateInfixExpression(infix_expr);
        }
        // Add other node types here...

        auto err = std::make_unique<YiniObject>();
        err->type = YiniObject::Type::ERROR;
        err->value = "Evaluation for this node type not yet implemented";
        return err;
    }

    std::unique_ptr<YiniObject> Evaluator::evaluateInfixExpression(AST::InfixExpression* node)
    {
        auto left = evaluate(node->left.get());
        auto right = evaluate(node->right.get());

        if (left->type == YiniObject::Type::INTEGER && right->type == YiniObject::Type::INTEGER)
        {
            int64_t left_val = std::get<int64_t>(left->value);
            int64_t right_val = std::get<int64_t>(right->value);

            auto result = std::make_unique<YiniObject>();
            result->type = YiniObject::Type::INTEGER;

            if (node->token.type == TokenType::PLUS)
            {
                result->value = left_val + right_val;
                return result;
            }
            else if (node->token.type == TokenType::MINUS)
            {
                result->value = left_val - right_val;
                return result;
            }
            else if (node->token.type == TokenType::STAR)
            {
                result->value = left_val * right_val;
                return result;
            }
            else if (node->token.type == TokenType::SLASH)
            {
                // Note: No division by zero check yet
                result->value = left_val / right_val;
                return result;
            }
        }

        auto err = std::make_unique<YiniObject>();
        err->type = YiniObject::Type::ERROR;
        err->value = "Cannot evaluate infix expression with these types";
        return err;
    }
}
