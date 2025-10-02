#include "Interpreter.h"
#include "Core/DynaValue.h"
#include "Core/YiniException.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>

namespace YINI
{
    std::string Interpreter::stringify(const std::any& value)
    {
        if (value.type() == typeid(nullptr)) return "null";

        if (value.type() == typeid(double)) {
            std::string str = std::to_string(std::any_cast<double>(value));
            // Remove trailing zeros
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') {
                str.pop_back();
            }
            return str;
        }

        if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        }

        if (value.type() == typeid(std::string)) {
            return "\"" + std::any_cast<std::string>(value) + "\"";
        }

        if (value.type() == typeid(std::vector<std::any>)) {
            std::stringstream ss;
            ss << "[";
            const auto& vec = std::any_cast<const std::vector<std::any>&>(value);
            for (size_t i = 0; i < vec.size(); ++i) {
                ss << stringify(vec[i]);
                if (i < vec.size() - 1) {
                    ss << ", ";
                }
            }
            ss << "]";
            return ss.str();
        }

        if (value.type() == typeid(std::map<std::string, std::any>)) {
            std::stringstream ss;
            ss << "{";
            const auto& map = std::any_cast<const std::map<std::string, std::any>&>(value);
            size_t i = 0;
            for (const auto& pair : map) {
                ss << "\"" << pair.first << "\": " << stringify(pair.second);
                if (i < map.size() - 1) {
                    ss << ", ";
                }
                i++;
            }
            ss << "}";
            return ss.str();
        }

        return "unsupported_type";
    }

    // Helper functions for type checking
    static bool is_number(const std::any& value) {
        return value.type() == typeid(double);
    }

    static void check_number_operand(const Token& op, const std::any& operand) {
        if (!is_number(operand)) {
            throw YiniException("Operand must be a number for operator '" + op.lexeme + "'.", op.line);
        }
    }

    static void check_number_operands(const Token& op, const std::any& left, const std::any& right) {
        if (!is_number(left) || !is_number(right)) {
            throw YiniException("Operands must be numbers for operator '" + op.lexeme + "'.", op.line);
        }
    }

    void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements)
    {
        // First pass: catalog sections and define macros
        for (const auto& statement : statements)
        {
            if (auto* define = dynamic_cast<Define*>(statement.get())) {
                execute(*define);
            } else if (auto* section = dynamic_cast<Section*>(statement.get())) {
                m_sections[section->name.lexeme] = section;
            }
        }

        // Second pass: resolve all sections
        for (const auto& pair : m_sections)
        {
            resolve_section(pair.second);
        }
    }

    void Interpreter::resolve_section(const Section* section)
    {
        if (m_resolved.count(section->name.lexeme)) {
            return; // Already resolved
        }
        if (m_resolving.count(section->name.lexeme)) {
            throw YiniException("Circular inheritance detected involving section '" + section->name.lexeme + "'.", section->name.line);
        }

        m_resolving.insert(section->name.lexeme);

        // Resolve parents first
        for (const auto& parent_token : section->parents)
        {
            if (!m_sections.count(parent_token.lexeme)) {
                throw YiniException("Parent section '" + parent_token.lexeme + "' not found.", parent_token.line);
            }
            resolve_section(m_sections.at(parent_token.lexeme));
        }

        // Create the section map and merge parent values
        resolved_sections[section->name.lexeme] = {};
        for (const auto& parent_token : section->parents)
        {
            const auto& parent_values = resolved_sections.at(parent_token.lexeme);
            for (const auto& pair : parent_values) {
                resolved_sections[section->name.lexeme][pair.first] = pair.second;
            }
        }

        // Execute this section's own statements to override inherited values
        m_current_section_name = section->name.lexeme;
        for (const auto& statement : section->statements)
        {
            execute(*statement);
        }

        m_resolving.erase(section->name.lexeme);
        m_resolved.insert(section->name.lexeme);
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
        resolved_sections[m_current_section_name][stmt.key.lexeme] = value;
    }

    void Interpreter::visit(const Section& stmt)
    {
        // The main logic is now in resolve_section, which is called from interpret.
        // This visit method is implicitly called by the main interpret loop.
    }

    void Interpreter::visit(const Register& stmt)
    {
        // This will be implemented more fully later. For now, just evaluate.
        evaluate(*stmt.value);
    }

    void Interpreter::visit(const Define& stmt)
    {
        for (const auto& value : stmt.values)
        {
            std::any evaluatedValue = evaluate(*value->value);
            m_globals.define(value->key.lexeme, evaluatedValue);
        }
    }

    void Interpreter::visit(const Include& stmt)
    {
        // This is handled by the file loader, so the interpreter can ignore it.
    }

    // Expression visitor methods
    std::any Interpreter::visit(const Literal& expr)
    {
        return expr.value;
    }

    std::any Interpreter::visit(const Variable& expr)
    {
        return m_globals.get(expr.name);
    }

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
                if (right_val == 0) throw YiniException("Division by zero.", expr.op.line);
                return left_val / right_val;
            case TokenType::PERCENT:
                if (right_val == 0) throw YiniException("Division by zero.", expr.op.line);
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
                throw YiniException("Map keys must be strings.", expr.brace.line);
            }
            std::string key = std::any_cast<std::string>(key_node->value);
            map[key] = evaluate(*pair.second);
        }
        return map;
    }

    std::any Interpreter::visit(const Call& expr)
    {
        auto callee = evaluate(*expr.callee);
        std::string callee_name;

        if (callee.type() == typeid(std::string)) {
            callee_name = std::any_cast<std::string>(callee);
        } else {
            throw YiniException("Can only call functions and constructors.", expr.paren.line);
        }

        if (callee_name == "Dyna" || callee_name == "dyna") {
            if (expr.arguments.size() != 1) {
                throw YiniException("Dyna() expects exactly one argument.", expr.paren.line);
            }
            std::any value = evaluate(*expr.arguments[0]);
            return DynaValue(value);
        }

        // Placeholder for other function calls
        return std::any{};
    }
}