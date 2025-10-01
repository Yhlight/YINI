#include "Interpreter.h"

namespace YINI
{
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
        // For now, we will just evaluate the expression.
        // In a later stage, we would store this in a data structure.
        evaluate(*stmt.value);
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
    std::any Interpreter::visit(const Unary& expr) { return std::any{}; }
    std::any Interpreter::visit(const Binary& expr) { return std::any{}; }
    std::any Interpreter::visit(const Grouping& expr) { return std::any{}; }
    std::any Interpreter::visit(const Array& expr) { return std::any{}; }
    std::any Interpreter::visit(const Set& expr) { return std::any{}; }
    std::any Interpreter::visit(const Map& expr) { return std::any{}; }
    std::any Interpreter::visit(const Call& expr) { return std::any{}; }
}