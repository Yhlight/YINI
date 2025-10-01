#include "Interpreter.h"
#include <cmath>
#include <stdexcept>

namespace YINI
{
    // Helper functions for type checking
    static bool is_number(const std::any& value) {
        return value.type() == typeid(double);
    }

    static void check_number_operand(const Token& op, const std::any& operand) {
        if (!is_number(operand)) {
            throw std::runtime_error("Operand must be a number for operator '" + op.lexeme + "'.");
        }
    }

    static void check_number_operands(const Token& op, const std::any& left, const std::any& right) {
        if (!is_number(left) || !is_number(right)) {
            throw std::runtime_error("Operands must be numbers for operator '" + op.lexeme + "'.");
        }
    }

    void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements)
    {
        for (const auto& statement : statements)
        {
            execute(*statement);
        }
    }

    void Interpreter::execute(const Stmt& stmt)
    {
        stmt.accept(*this);
    }

    std::any Interpreter::evaluate(const Expr& expr)
    {
        return expr.accept(*this);
    }

    // Statement visitor methods
    void Interpreter::visit(const KeyValue& stmt)
    {
        std::any value = evaluate(*stmt.value);
        results[stmt.key.lexeme] = value;
    }

    void Interpreter::visit(const Section& stmt)
    {
        // Execute all statements within the section.
        for (const auto& statement : stmt.statements)
        {
            execute(*statement);
        }
    }

    void Interpreter::visit(const Register& stmt)
    {
        // Similar to KeyValue, just evaluate for now.
        evaluate(*stmt.value);
    }

    void Interpreter::visit(const Define& stmt)
    {
        // Define all key-value pairs in the environment.
        for (const auto& value : stmt.values)
        {
            std::any evaluatedValue = evaluate(*value->value);
            m_environment.define(value->key.lexeme, evaluatedValue);
        }
    }

    // Expression visitor methods
    std::any Interpreter::visit(const Literal& expr)
    {
        return expr.value;
    }

    std::any Interpreter::visit(const Variable& expr)
    {
        return m_environment.get(expr.name);
    }

    // Placeholders for expressions not yet fully implemented
    std::any Interpreter::visit(const Grouping& expr)
    {
        return evaluate(*expr.expression);
    }

    std::any Interpreter::visit(const Unary& expr)
    {
        std::any right = evaluate(*expr.right);
        check_number_operand(expr.op, right);

        if (expr.op.type == TokenType::MINUS)
        {
            return -std::any_cast<double>(right);
        }
        return std::any{}; // Unreachable
    }

    std::any Interpreter::visit(const Binary& expr)
    {
        std::any left = evaluate(*expr.left);
        std::any right = evaluate(*expr.right);

        check_number_operands(expr.op, left, right);

        double left_val = std::any_cast<double>(left);
        double right_val = std::any_cast<double>(right);

        switch (expr.op.type)
        {
            case TokenType::PLUS:
                return left_val + right_val;
            case TokenType::MINUS:
                return left_val - right_val;
            case TokenType::STAR:
                return left_val * right_val;
            case TokenType::SLASH:
                if (right_val == 0) throw std::runtime_error("Division by zero.");
                return left_val / right_val;
            case TokenType::PERCENT:
                if (right_val == 0) throw std::runtime_error("Division by zero.");
                return fmod(left_val, right_val);
            default:
                break;
        }

        return std::any{}; // Unreachable
    }

    std::any Interpreter::visit(const Array& expr)
    {
        std::vector<std::any> elements;
        for (const auto& element : expr.elements)
        {
            elements.push_back(evaluate(*element));
        }
        return elements;
    }

    std::any Interpreter::visit(const Set& expr)
    {
        std::vector<std::any> elements;
        for (const auto& element : expr.elements)
        {
            elements.push_back(evaluate(*element));
        }
        return elements;
    }

    std::any Interpreter::visit(const Map& expr)
    {
        std::map<std::string, std::any> map;
        for (const auto& pair : expr.pairs)
        {
            // For now, we assume keys are simple strings (literals or identifiers)
            // A more robust implementation would evaluate the key expression
            auto key_node = dynamic_cast<Literal*>(pair.first.get());
            if (!key_node || key_node->value.type() != typeid(std::string)) {
                throw std::runtime_error("Map keys must be strings.");
            }
            std::string key = std::any_cast<std::string>(key_node->value);
            map[key] = evaluate(*pair.second);
        }
        return map;
    }

    std::any Interpreter::visit(const Call& expr)
    {
        // Evaluate arguments, but do nothing with them for now.
        // This lays the groundwork for built-in functions.
        for (const auto& argument : expr.arguments)
        {
            evaluate(*argument);
        }
        return std::any{}; // Placeholder for call result
    }
}