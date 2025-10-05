#include "Interpreter.h"
#include "Core/DynaValue.h"
#include "Core/YiniException.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>
#include <variant>
#include <cstdlib> // For getenv

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
        // Clear state for potential reuse of the interpreter instance
        m_sections.clear();
        m_globals.clear();
        m_expression_map.clear();
        m_kv_map.clear();
        m_resolved.clear();
        m_resolving.clear();
        resolved_sections.clear();
        value_locations.clear();
        m_currently_resolving_values.clear();

        // 1. Discovery Pass: Find all sections and global definitions
        for (const auto& statement : statements)
        {
            if (auto* define = dynamic_cast<Define*>(statement.get())) {
                execute(*define); // Populates m_globals
            } else if (auto* section = dynamic_cast<Section*>(statement.get())) {
                if (m_sections.count(section->name.lexeme)) {
                    throw RuntimeError("Section '" + section->name.lexeme + "' has already been defined.", section->name.line, section->name.column, section->name.filepath);
                }
                m_sections[section->name.lexeme] = section;
            }
        }

        // 2. Mapping Pass: Build the expression map, handling inheritance
        for (const auto& pair : m_sections)
        {
            build_expression_map(pair.second);
        }

        // 3. Evaluation Pass: Resolve all expressions
        for (const auto& section_pair : m_expression_map) {
            const std::string& section_name = section_pair.first;
            for (const auto& key_pair : section_pair.second) {
                // This will trigger the recursive, on-demand evaluation for each key
                // Create dummy tokens as they are not tied to a specific source location
                visit(XRef(
                    Token{TokenType::IDENTIFIER, section_name, {}, 0, 0, ""},
                    Token{TokenType::IDENTIFIER, key_pair.first, {}, 0, 0, ""}
                ));
            }
        }
    }

    void Interpreter::build_expression_map(const Section* section)
    {
        // Check for inheritance cycles
        if (m_resolved.count(section->name.lexeme)) return;
        if (m_resolving.count(section->name.lexeme)) {
            throw RuntimeError("Circular inheritance detected involving section '" + section->name.lexeme + "'.", section->name.line, section->name.column, section->name.filepath);
        }

        m_resolving.insert(section->name.lexeme);

        // Recursively build maps for parent sections first
        for (const auto& parent_token : section->parents)
        {
            if (!m_sections.count(parent_token.lexeme)) {
                throw RuntimeError("Parent section '" + parent_token.lexeme + "' not found.", parent_token.line, parent_token.column, parent_token.filepath);
            }
            build_expression_map(m_sections.at(parent_token.lexeme));
        }

        // Inherit expressions from parents
        if (!m_expression_map.count(section->name.lexeme)) {
            m_expression_map[section->name.lexeme] = {};
            m_kv_map[section->name.lexeme] = {};
        }
        for (const auto& parent_token : section->parents)
        {
            const auto& parent_exprs = m_expression_map.at(parent_token.lexeme);
            for (const auto& pair : parent_exprs) {
                m_expression_map[section->name.lexeme][pair.first] = pair.second;
                m_kv_map[section->name.lexeme][pair.first] = m_kv_map.at(parent_token.lexeme).at(pair.first);
            }
        }

        // Add expressions from the current section, overriding parents
        for (const auto& statement : section->statements)
        {
            if (auto* kv = dynamic_cast<const KeyValue*>(statement.get())) {
                m_expression_map[section->name.lexeme][kv->key.lexeme] = kv->value.get();
                m_kv_map[section->name.lexeme][kv->key.lexeme] = kv;
            }
        }

        m_resolving.erase(section->name.lexeme);
        m_resolved.insert(section->name.lexeme);
    }

    void Interpreter::execute(const Stmt& stmt) { stmt.accept(*this); }
    YiniValue Interpreter::evaluate(const Expr& expr) { return expr.accept(*this); }

    void Interpreter::visit([[maybe_unused]] const KeyValue& stmt) {}
    void Interpreter::visit([[maybe_unused]] const Section& stmt) {}
    void Interpreter::visit([[maybe_unused]] const Register& stmt) {}
    void Interpreter::visit([[maybe_unused]] const Include& stmt) {}
    void Interpreter::visit([[maybe_unused]] const Schema& stmt) {}

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

    YiniValue Interpreter::visit(const EnvVariable& expr)
    {
        const char* value = std::getenv(expr.name.lexeme.c_str());
        if (value != nullptr)
        {
            return std::string(value);
        }

        if (expr.default_value != nullptr)
        {
            return evaluate(*expr.default_value);
        }

        throw RuntimeError("Required environment variable '" + expr.name.lexeme + "' is not set and no default value is provided.", expr.name.line, expr.name.column, expr.name.filepath);
    }

    YiniValue Interpreter::visit(const XRef& expr) {
        std::string section_name = expr.section.lexeme;
        std::string key_name = expr.key.lexeme;
        std::string full_ref = section_name + "." + key_name;

        // If value is already resolved, return it from the cache
        if (resolved_sections.count(section_name) && resolved_sections.at(section_name).count(key_name)) {
            return resolved_sections.at(section_name).at(key_name);
        }

        // Check for circular references
        if (m_currently_resolving_values.count(full_ref)) {
            throw RuntimeError("Circular reference detected for value '" + full_ref + "'.", expr.section.line, expr.section.column, expr.section.filepath);
        }

        // Check if the expression to be resolved exists in our map
        if (!m_expression_map.count(section_name) || !m_expression_map.at(section_name).count(key_name)) {
            throw RuntimeError("Referenced key '" + key_name + "' not found in section '" + section_name + "'.", expr.key.line, expr.key.column, expr.key.filepath);
        }

        m_currently_resolving_values.insert(full_ref);

        // Recursively evaluate the expression
        const Expr* expr_to_eval = m_expression_map.at(section_name).at(key_name);
        YiniValue result = evaluate(*expr_to_eval);

        // Cache the result and its location
        resolved_sections[section_name][key_name] = result;
        const auto* kv_stmt = m_kv_map.at(section_name).at(key_name);
        value_locations[section_name][key_name] = {kv_stmt->value_line, kv_stmt->value_column};

        m_currently_resolving_values.erase(full_ref);

        return result;
    }

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