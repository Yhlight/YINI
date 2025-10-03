#include "Interpreter.h"
#include "Core/DynaValue.h"
#include "Core/YiniException.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>
#include <variant>

namespace YINI
{
    // Visitor for the stringify function
    struct StringifyVisitor
    {
        Interpreter& interpreter;

        std::string operator()(std::monostate) const { return "null"; }
        std::string operator()(bool value) const { return value ? "true" : "false"; }
        std::string operator()(double value) const {
            std::string str = std::to_string(value);
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.back() == '.') {
                str.pop_back();
            }
            return str;
        }
        std::string operator()(const std::string& value) const { return "\"" + value + "\""; }
        std::string operator()(const std::unique_ptr<YiniArray>& value) const {
            std::stringstream ss;
            ss << "[";
            for (size_t i = 0; i < value->size(); ++i) {
                ss << interpreter.stringify((*value)[i]);
                if (i < value->size() - 1) {
                    ss << ", ";
                }
            }
            ss << "]";
            return ss.str();
        }
        std::string operator()(const std::unique_ptr<YiniMap>& value) const {
            std::stringstream ss;
            ss << "{";
            size_t i = 0;
            for (const auto& pair : *value) {
                ss << "\"" << pair.first << "\": " << interpreter.stringify(pair.second);
                if (i < value->size() - 1) {
                    ss << ", ";
                }
                i++;
            }
            ss << "}";
            return ss.str();
        }
        std::string operator()(const std::unique_ptr<DynaValue>& value) const {
             return "Dyna(" + interpreter.stringify(*value->m_value) + ")";
        }
    };

    std::string Interpreter::stringify(const YiniValue& value)
    {
        return std::visit(StringifyVisitor{*this}, value.m_value);
    }

    // Helper functions for type checking
    static bool is_number(const YiniValue& value) {
        return std::holds_alternative<double>(value.m_value);
    }

    static void check_number_operand(const Token& op, const YiniValue& operand) {
        if (!is_number(operand)) {
            throw RuntimeError("Operand must be a number for operator '" + op.lexeme + "'.", op.line, op.column, op.filepath);
        }
    }

    static void check_number_operands(const Token& op, const YiniValue& left, const YiniValue& right) {
        if (!is_number(left) || !is_number(right)) {
            throw RuntimeError("Operands must be numbers for operator '" + op.lexeme + "'.", op.line, op.column, op.filepath);
        }
    }

    void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements)
    {
        for (const auto& statement : statements)
        {
            if (auto* define = dynamic_cast<Define*>(statement.get())) {
                execute(*define);
            } else if (auto* section = dynamic_cast<Section*>(statement.get())) {
                m_sections[section->name.lexeme] = section;
            }
        }

        for (const auto& pair : m_sections)
        {
            resolve_section(pair.second);
        }
    }

    void Interpreter::resolve_section(const Section* section)
    {
        if (m_resolved.count(section->name.lexeme)) return;
        if (m_resolving.count(section->name.lexeme)) {
            throw RuntimeError("Circular inheritance detected involving section '" + section->name.lexeme + "'.", section->name.line, section->name.column, section->name.filepath);
        }

        m_resolving.insert(section->name.lexeme);

        for (const auto& parent_token : section->parents)
        {
            if (!m_sections.count(parent_token.lexeme)) {
                throw RuntimeError("Parent section '" + parent_token.lexeme + "' not found.", parent_token.line, parent_token.column, parent_token.filepath);
            }
            resolve_section(m_sections.at(parent_token.lexeme));
        }

        resolved_sections[section->name.lexeme] = {};
        for (const auto& parent_token : section->parents)
        {
            const auto& parent_values = resolved_sections.at(parent_token.lexeme);
            for (const auto& pair : parent_values) {
                resolved_sections[section->name.lexeme][pair.first] = pair.second;
            }
        }

        m_current_section_name = section->name.lexeme;
        for (const auto& statement : section->statements)
        {
            execute(*statement);
        }

        m_resolving.erase(section->name.lexeme);
        m_resolved.insert(section->name.lexeme);
    }

    void Interpreter::execute(const Stmt& stmt) { stmt.accept(*this); }
    YiniValue Interpreter::evaluate(const Expr& expr) { return expr.accept(*this); }

    void Interpreter::visit(const KeyValue& stmt)
    {
        YiniValue value = evaluate(*stmt.value);
        resolved_sections[m_current_section_name][stmt.key.lexeme] = value;
        value_locations[m_current_section_name][stmt.key.lexeme] = {stmt.value_line, stmt.value_column};
    }

    void Interpreter::visit(const Section& stmt) {} // Handled in resolve_section
    void Interpreter::visit(const Register& stmt) { evaluate(*stmt.value); }
    void Interpreter::visit(const Include& stmt) {} // Handled by file loader

    void Interpreter::visit(const Define& stmt)
    {
        for (const auto& value : stmt.values)
        {
            m_globals.define(value->key.lexeme, evaluate(*value->value));
        }
    }

    YiniValue Interpreter::visit(const Literal& expr) { return expr.value; }
    YiniValue Interpreter::visit(const Variable& expr) { return m_globals.get(expr.name); }
    YiniValue Interpreter::visit(const Grouping& expr) { return evaluate(*expr.expression); }

    YiniValue Interpreter::visit(const Unary& expr)
    {
        YiniValue right = evaluate(*expr.right);
        check_number_operand(expr.op, right);
        return -std::get<double>(right.m_value);
    }

    YiniValue Interpreter::visit(const Binary& expr)
    {
        YiniValue left = evaluate(*expr.left);
        YiniValue right = evaluate(*expr.right);
        check_number_operands(expr.op, left, right);
        double left_val = std::get<double>(left.m_value);
        double right_val = std::get<double>(right.m_value);

        switch (expr.op.type)
        {
            case TokenType::PLUS: return left_val + right_val;
            case TokenType::MINUS: return left_val - right_val;
            case TokenType::STAR: return left_val * right_val;
            case TokenType::SLASH:
                if (right_val == 0) throw RuntimeError("Division by zero.", expr.op.line, expr.op.column, expr.op.filepath);
                return left_val / right_val;
            case TokenType::PERCENT:
                if (right_val == 0) throw RuntimeError("Division by zero.", expr.op.line, expr.op.column, expr.op.filepath);
                return fmod(left_val, right_val);
            default: break;
        }
        return {}; // Unreachable
    }

    YiniValue Interpreter::visit(const Array& expr)
    {
        YiniArray elements;
        for (const auto& element : expr.elements)
        {
            elements.push_back(evaluate(*element));
        }
        return YiniValue(std::move(elements));
    }

    YiniValue Interpreter::visit(const Set& expr)
    {
        YiniArray elements;
        for (const auto& element : expr.elements)
        {
            elements.push_back(evaluate(*element));
        }
        return YiniValue(std::move(elements));
    }

    YiniValue Interpreter::visit(const Map& expr)
    {
        YiniMap map;
        for (const auto& pair : expr.pairs)
        {
            YiniValue key_val = evaluate(*pair.first);
            if (!std::holds_alternative<std::string>(key_val.m_value)) {
                throw RuntimeError("Map keys must evaluate to strings.", expr.brace.line, expr.brace.column, expr.brace.filepath);
            }
            map[std::get<std::string>(key_val.m_value)] = evaluate(*pair.second);
        }
        return YiniValue(std::move(map));
    }

    YiniValue Interpreter::visit(const Call& expr)
    {
        auto callee = evaluate(*expr.callee);
        if (!std::holds_alternative<std::string>(callee.m_value)) {
            throw YiniException("Can only call functions by name.", expr.paren.line, expr.paren.column, expr.paren.filepath);
        }
        std::string callee_name = std::get<std::string>(callee.m_value);

        if (callee_name == "Dyna" || callee_name == "dyna") {
            if (expr.arguments.size() != 1) {
                throw YiniException("Dyna() expects exactly one argument.", expr.paren.line, expr.paren.column, expr.paren.filepath);
            }
            return YiniValue(DynaValue(evaluate(*expr.arguments[0])));
        }
        return {}; // Placeholder for other functions
    }
}