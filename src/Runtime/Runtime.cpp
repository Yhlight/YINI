#include "Runtime.h"
#include <stdexcept>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

namespace Yini
{
    static std::string toLower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); }); return s; }

    void YiniRuntime::load(Ast::YiniDocument* doc)
    {
        for (const auto& stmt : doc->statements) {
            if (auto* section = dynamic_cast<Ast::Section*>(stmt.get())) {
                std::string section_name = section->name->value;
                if (section_name == "#define") {
                    for (const auto& define_stmt : section->statements)
                        if (auto* kvp = dynamic_cast<Ast::KeyValuePair*>(define_stmt.get())) m_define_asts[kvp->key->value] = kvp->value.get();
                } else {
                    SectionAst section_ast;
                    for (const auto& section_stmt : section->statements) {
                        if (auto* kvp = dynamic_cast<Ast::KeyValuePair*>(section_stmt.get())) {
                            if (auto* call = dynamic_cast<Ast::FunctionCall*>(kvp->value.get())) {
                                if (toLower(call->functionName->value) == "dyna" && !call->arguments.empty()) {
                                    m_dynamic_keys.insert(section_name + "." + kvp->key->value);
                                    section_ast[kvp->key->value] = call->arguments[0].get();
                                    continue;
                                }
                            }
                            section_ast[kvp->key->value] = kvp->value.get();
                        }
                    }
                    m_section_asts[section_name] = section_ast;
                }
            }
        }
        for (const auto& stmt : doc->statements) {
            if (auto* section = dynamic_cast<Ast::Section*>(stmt.get())) {
                if (!section->parents.empty()) {
                    for (const auto& parent : section->parents) {
                        if (m_section_asts.count(parent->value)) {
                            for (const auto& [key, expr] : m_section_asts.at(parent->value)) {
                                if (m_section_asts.at(section->name->value).find(key) == m_section_asts.at(section->name->value).end())
                                    m_section_asts.at(section->name->value)[key] = expr;
                            }
                        }
                    }
                }
            }
        }
    }

    const std::vector<YiniError>& YiniRuntime::getErrors() const { return m_runtime_errors; }

    std::shared_ptr<Value> YiniRuntime::getValue(const std::string& sectionName, const std::string& key)
    {
        if (m_section_cache.count(sectionName) && m_section_cache.at(sectionName).count(key)) return m_section_cache.at(sectionName).at(key);
        if (sectionName.empty() && m_define_cache.count(key)) return m_define_cache.at(key);

        Ast::Expression* expr = nullptr;
        std::string foundInSection = sectionName;

        if (m_section_asts.count(sectionName) && m_section_asts.at(sectionName).count(key)) {
            expr = m_section_asts.at(sectionName).at(key);
        }

        if (!expr && m_define_asts.count(key)) {
             expr = m_define_asts.at(key);
             foundInSection = "";
        }

        if (!expr) {
            m_runtime_errors.emplace_back(ErrorType::Runtime, "Variable '" + key + "' not found in section '" + sectionName + "'.");
            return nullptr;
        }

        auto value = evaluateExpression(expr, foundInSection);

        if (value) {
            if (foundInSection.empty()) m_define_cache[key] = value;
            else m_section_cache[foundInSection][key] = value;
        }
        return value;
    }

    std::shared_ptr<Value> YiniRuntime::evaluateExpression(Ast::Expression* expr, const std::string& sectionScope) { return visit(expr, sectionScope); }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Expression* node, const std::string& sectionScope) const {
        if (!node) return nullptr;
        if (auto* n = dynamic_cast<Ast::Identifier*>(node)) return visit(n, sectionScope);
        if (auto* n = dynamic_cast<Ast::IntegerLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::FloatLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::BooleanLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::StringLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::ColorLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::InfixExpression*>(node)) return visit(n, sectionScope);
        if (auto* n = dynamic_cast<Ast::MacroReference*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::ArrayLiteral*>(node)) return visit(n, sectionScope);
        if (auto* n = dynamic_cast<Ast::MapLiteral*>(node)) return visit(n, sectionScope);
        if (auto* n = dynamic_cast<Ast::FunctionCall*>(node)) return visit(n, sectionScope);
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::IntegerLiteral* node) const { auto v=std::make_shared<Value>(); v->data.emplace<Integer>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::FloatLiteral* node) const { auto v=std::make_shared<Value>(); v->data.emplace<Float>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::BooleanLiteral* node) const { auto v=std::make_shared<Value>(); v->data.emplace<Boolean>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::StringLiteral* node) const { auto v=std::make_shared<Value>(); v->data.emplace<String>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::ColorLiteral* node) const { auto v=std::make_shared<Value>(); v->data.emplace<String>(node->token.literal); return v; }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Identifier* node, const std::string& sectionScope) const { return const_cast<YiniRuntime*>(this)->getValue(sectionScope, node->value); }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::MacroReference* node) const { return const_cast<YiniRuntime*>(this)->getValue("", node->name->value); }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::InfixExpression* node, const std::string& sectionScope) const {
        auto left = const_cast<YiniRuntime*>(this)->evaluateExpression(node->left.get(), sectionScope);
        auto right = const_cast<YiniRuntime*>(this)->evaluateExpression(node->right.get(), sectionScope);
        if (!left || !right) return nullptr;
        auto result = std::make_shared<Value>();
        if (std::holds_alternative<Float>(left->data) || std::holds_alternative<Float>(right->data)) {
            double leftVal = std::holds_alternative<Float>(left->data) ? std::get<Float>(left->data) : (double)std::get<Integer>(left->data);
            double rightVal = std::holds_alternative<Float>(right->data) ? std::get<Float>(right->data) : (double)std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Float>(leftVal + rightVal);
            else if (node->op == "-") result->data.emplace<Float>(leftVal - rightVal);
            else if (node->op == "*") result->data.emplace<Float>(leftVal * rightVal);
            else if (node->op == "/") result->data.emplace<Float>(leftVal / rightVal);
        } else if (std::holds_alternative<Integer>(left->data) && std::holds_alternative<Integer>(right->data)) {
            long long leftVal = std::get<Integer>(left->data);
            long long rightVal = std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Integer>(leftVal + rightVal);
            else if (node->op == "-") result->data.emplace<Integer>(leftVal - rightVal);
            else if (node->op == "*") result->data.emplace<Integer>(leftVal * rightVal);
            else if (node->op == "/") result->data.emplace<Integer>(leftVal / rightVal);
            else if (node->op == "%") result->data.emplace<Integer>(leftVal % rightVal);
        }
        return result;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::ArrayLiteral* node, const std::string& sectionScope) const {
        auto val = std::make_shared<Value>();
        Array arr;
        for(const auto& elem_node : node->elements) {
            arr.push_back(const_cast<YiniRuntime*>(this)->evaluateExpression(elem_node.get(), sectionScope));
        }
        val->data.emplace<Array>(arr);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::MapLiteral* node, const std::string& sectionScope) const {
        auto val = std::make_shared<Value>();
        Map map;
        for(const auto& elem_node : node->elements) {
            auto value = const_cast<YiniRuntime*>(this)->evaluateExpression(elem_node->value.get(), sectionScope);
            if (value) map[elem_node->key->value] = value;
        }
        val->data.emplace<Map>(map);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::FunctionCall* node, const std::string& sectionScope) const {
        auto val = std::make_shared<Value>();
        std::string funcName = toLower(node->functionName->value);
        if (funcName == "coord") {
            if (node->arguments.size() < 2 || node->arguments.size() > 3) return nullptr;
            auto x_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[0].get(), sectionScope);
            auto y_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[1].get(), sectionScope);
            if (!x_val || !y_val) return nullptr;
            Coord c;
            c.x = std::holds_alternative<Float>(x_val->data) ? std::get<Float>(x_val->data) : (double)std::get<Integer>(x_val->data);
            c.y = std::holds_alternative<Float>(y_val->data) ? std::get<Float>(y_val->data) : (double)std::get<Integer>(y_val->data);
            if (node->arguments.size() == 3) {
                auto z_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[2].get(), sectionScope);
                if (!z_val) return nullptr;
                c.z = std::holds_alternative<Float>(z_val->data) ? std::get<Float>(z_val->data) : (double)std::get<Integer>(z_val->data);
                c.is_3d = true;
            }
            val->data.emplace<Coord>(c);
        } else if (funcName == "color") {
            if (node->arguments.size() != 3) return nullptr;
            auto r_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[0].get(), sectionScope);
            auto g_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[1].get(), sectionScope);
            auto b_val = const_cast<YiniRuntime*>(this)->evaluateExpression(node->arguments[2].get(), sectionScope);
            if (!r_val || !g_val || !b_val || !std::holds_alternative<Integer>(r_val->data) || !std::holds_alternative<Integer>(g_val->data) || !std::holds_alternative<Integer>(b_val->data)) return nullptr;
            Color c;
            c.r = static_cast<uint8_t>(std::get<Integer>(r_val->data));
            c.g = static_cast<uint8_t>(std::get<Integer>(g_val->data));
            c.b = static_cast<uint8_t>(std::get<Integer>(b_val->data));
            val->data.emplace<Color>(c);
        } else if (funcName == "path") {
             val->data.emplace<String>("path()");
        }
        return val;
    }

    bool YiniRuntime::setValue(const std::string& sectionName, const std::string& key, std::shared_ptr<Value> value) {
        if (m_dynamic_keys.find(sectionName + "." + key) == m_dynamic_keys.end()) {
            m_runtime_errors.emplace_back(ErrorType::Runtime, "Attempted to set value for non-dynamic key '" + key + "'.");
            return false;
        }
        m_section_cache[sectionName][key] = value;
        return true;
    }

    bool YiniRuntime::serialize(const std::string& filepath) { return false; }
    bool YiniRuntime::deserialize(const std::string& filepath) { return false; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::Node* node, const std::string& sectionScope) const { return nullptr; }
}
