#include "Runtime.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <stdexcept>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

namespace Yini
{
    static std::string readFileIntoString(const std::string& filepath) { std::ifstream file(filepath); if (!file) return ""; std::stringstream buffer; buffer << file.rdbuf(); return buffer.str(); }
    static std::string toLower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); }); return s; }

    void YiniRuntime::loadAndMerge(const std::string& filepath, std::set<std::string>& processedFiles, std::vector<YiniError>& aggregated_errors) {
        if (processedFiles.count(filepath)) return;
        processedFiles.insert(filepath);
        std::string content = readFileIntoString(filepath);
        if (content.empty()) return;
        Lexer lexer(content);
        Parser parser(lexer);
        auto doc = parser.parseDocument();
        aggregated_errors.insert(aggregated_errors.end(), parser.getErrors().begin(), parser.getErrors().end());
        if (!parser.getErrors().empty()) { throw std::runtime_error("Parsing failed for file: " + filepath); }
        for (const auto& stmt : doc->statements) {
            if (auto* section = dynamic_cast<Ast::Section*>(stmt.get())) {
                if (section->name->value == "#include") {
                    for (const auto& include_stmt_base : section->statements) {
                        if (auto* include_stmt = dynamic_cast<Ast::IncludeStatement*>(include_stmt_base.get())) {
                            if (auto* path_literal = dynamic_cast<Ast::StringLiteral*>(include_stmt->filepath.get())) {
                                loadAndMerge(path_literal->value, processedFiles, aggregated_errors);
                            }
                        }
                    }
                }
            }
        }
        evaluate(doc.get());
    }

    bool YiniRuntime::loadFromFile(const std::string& filepath) {
        m_runtime_errors.clear();
        try {
            std::set<std::string> processedFiles;
            loadAndMerge(filepath, processedFiles, m_runtime_errors);
        } catch (const std::exception& e) {
            m_runtime_errors.emplace_back(ErrorType::Runtime, e.what());
            return false;
        }
        return m_runtime_errors.empty();
    }

    void YiniRuntime::loadFromString(const std::string& content) {
        Lexer lexer(content);
        Parser parser(lexer);
        auto doc = parser.parseDocument();
        m_runtime_errors = parser.getErrors();
        if (m_runtime_errors.empty()) {
            evaluate(doc.get());
        }
    }

    void YiniRuntime::evaluate(Ast::YiniDocument* doc) {
        for (const auto& stmt : doc->statements) if (auto* section = dynamic_cast<Ast::Section*>(stmt.get())) if (section->name->value == "#define") visit(section);
        for (const auto& stmt : doc->statements) if (auto* section = dynamic_cast<Ast::Section*>(stmt.get())) if (section->name->value != "#define") visit(section);
    }

    const std::vector<YiniError>& YiniRuntime::getErrors() const { return m_runtime_errors; }
    std::shared_ptr<Value> YiniRuntime::getValue(const std::string& sectionName, const std::string& key) const {
        if (m_sections.count(sectionName) && m_sections.at(sectionName).count(key)) return m_sections.at(sectionName).at(key);
        if (m_defines.count(key)) return m_defines.at(key);
        return nullptr;
    }

    bool YiniRuntime::setValue(const std::string& sectionName, const std::string& key, std::shared_ptr<Value> value) {
        std::string dynamic_key = sectionName + "." + key;
        if (m_dynamic_keys.find(dynamic_key) == m_dynamic_keys.end()) {
            m_runtime_errors.emplace_back(ErrorType::Runtime, "Attempted to set non-dynamic key '" + key + "'.");
            return false;
        }
        auto current_value = getValue(sectionName, key);
        if (current_value) {
            auto& history = m_dynamic_history[dynamic_key];
            history.push_back(current_value);
            if (history.size() > 5) history.erase(history.begin());
        }
        m_sections[sectionName][key] = value;
        return true;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Node* node) {
        if (auto* n = dynamic_cast<Ast::Expression*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::Section*>(node)) visit(n);
        else if (auto* n = dynamic_cast<Ast::KeyValuePair*>(node)) visit(n);
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Expression* node) {
        if (!node) return nullptr;
        if (auto* n = dynamic_cast<Ast::IntegerLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::FloatLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::BooleanLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::StringLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::ColorLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::InfixExpression*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::MacroReference*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::ArrayLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::MapLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::FunctionCall*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::Identifier*>(node)) {
            m_runtime_errors.emplace_back(ErrorType::Runtime, "Cannot use key-to-key reference. Only @macros are allowed.", n->token.line, n->token.column);
            return nullptr;
        }
        return nullptr;
    }

    void YiniRuntime::visit(Ast::Section* node) {
        m_currentSectionName = node->name->value;
        auto& currentSectionData = m_sections[m_currentSectionName];
        for (const auto& parent : node->parents) if (m_sections.count(parent->value)) for (const auto& [key, val] : m_sections.at(parent->value)) currentSectionData[key] = val;
        for (const auto& stmt : node->statements) visit(stmt.get());
    }

    void YiniRuntime::visit(Ast::KeyValuePair* node) {
        if (auto* call = dynamic_cast<Ast::FunctionCall*>(node->value.get())) {
            if (toLower(call->functionName->value) == "dyna" && !call->arguments.empty()) {
                m_dynamic_keys.insert(m_currentSectionName + "." + node->key->value);
                auto value = visit(call->arguments[0].get());
                if (value) m_sections[m_currentSectionName][node->key->value] = value;
                return;
            }
        }
        auto value = visit(node->value.get());
        if (value) {
            if (m_currentSectionName == "#define") m_defines[node->key->value] = value;
            else m_sections[m_currentSectionName][node->key->value] = value;
        }
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::InfixExpression* node) {
        auto left = visit(node->left.get());
        auto right = visit(node->right.get());
        if (!left || !right) return nullptr;
        bool isLeftNum = std::holds_alternative<Integer>(left->data) || std::holds_alternative<Float>(left->data);
        bool isRightNum = std::holds_alternative<Integer>(right->data) || std::holds_alternative<Float>(right->data);
        if (!isLeftNum || !isRightNum) {
            m_runtime_errors.emplace_back(ErrorType::Type, "Arithmetic on non-numeric types.", node->token.line, node->token.column);
            return nullptr;
        }
        auto result = std::make_shared<Value>();
        if (std::holds_alternative<Float>(left->data) || std::holds_alternative<Float>(right->data)) {
            double l = std::holds_alternative<Float>(left->data) ? std::get<Float>(left->data) : (double)std::get<Integer>(left->data);
            double r = std::holds_alternative<Float>(right->data) ? std::get<Float>(right->data) : (double)std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Float>(l + r); else if (node->op == "-") result->data.emplace<Float>(l - r);
            else if (node->op == "*") result->data.emplace<Float>(l * r); else if (node->op == "/") result->data.emplace<Float>(l / r);
        } else {
            long long l = std::get<Integer>(left->data); long long r = std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Integer>(l + r); else if (node->op == "-") result->data.emplace<Integer>(l - r);
            else if (node->op == "*") result->data.emplace<Integer>(l * r); else if (node->op == "/") result->data.emplace<Integer>(l / r);
            else if (node->op == "%") result->data.emplace<Integer>(l % r);
        }
        return result;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::MacroReference* node) {
        if (m_defines.count(node->name->value)) return m_defines.at(node->name->value);
        std::stringstream ss; ss << "Runtime Error (L" << node->token.line << ":" << node->token.column << "): Macro '@" << node->name->value << "' not found.";
        throw std::runtime_error(ss.str());
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::IntegerLiteral* node) { auto v=std::make_shared<Value>(); v->data.emplace<Integer>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::FloatLiteral* node) { auto v=std::make_shared<Value>(); v->data.emplace<Float>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::BooleanLiteral* node) { auto v=std::make_shared<Value>(); v->data.emplace<Boolean>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::StringLiteral* node) { auto v=std::make_shared<Value>(); v->data.emplace<String>(node->value); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::ColorLiteral* node) { auto v=std::make_shared<Value>(); v->data.emplace<String>(node->token.literal); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::ArrayLiteral* node) { auto v=std::make_shared<Value>(); Array a; for(const auto& e:node->elements)a.push_back(visit(e.get())); v->data.emplace<Array>(a); return v; }
    std::shared_ptr<Value> YiniRuntime::visit(Ast::MapLiteral* node) { auto v=std::make_shared<Value>(); Map m; for(const auto& e:node->elements){auto V=visit(e->value.get());if(V)m[e->key->value]=V;} v->data.emplace<Map>(m); return v; }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::FunctionCall* node) {
        auto val = std::make_shared<Value>();
        std::string funcName = toLower(node->functionName->value);
        if (funcName == "coord") {
            if (node->arguments.size() < 2 || node->arguments.size() > 3) { m_runtime_errors.emplace_back(ErrorType::Type, "Coord expects 2 or 3 arguments.", node->token.line, node->token.column); return nullptr; }
            auto x_val = visit(node->arguments[0].get()); auto y_val = visit(node->arguments[1].get());
            if (!x_val || !y_val || !(std::holds_alternative<Integer>(x_val->data) || std::holds_alternative<Float>(x_val->data)) || !(std::holds_alternative<Integer>(y_val->data) || std::holds_alternative<Float>(y_val->data))) { m_runtime_errors.emplace_back(ErrorType::Type, "Coord arguments must be numeric.", node->token.line, node->token.column); return nullptr; }
            Coord c;
            c.x = std::holds_alternative<Float>(x_val->data) ? std::get<Float>(x_val->data) : (double)std::get<Integer>(x_val->data);
            c.y = std::holds_alternative<Float>(y_val->data) ? std::get<Float>(y_val->data) : (double)std::get<Integer>(y_val->data);
            if (node->arguments.size() == 3) {
                auto z_val = visit(node->arguments[2].get());
                if (!z_val || !(std::holds_alternative<Integer>(z_val->data) || std::holds_alternative<Float>(z_val->data))) { m_runtime_errors.emplace_back(ErrorType::Type, "Coord Z argument must be numeric.", node->token.line, node->token.column); return nullptr; }
                c.z = std::holds_alternative<Float>(z_val->data) ? std::get<Float>(z_val->data) : (double)std::get<Integer>(z_val->data);
                c.is_3d = true;
            }
            val->data.emplace<Coord>(c);
        } else if (funcName == "color") {
            if (node->arguments.size() != 3) { m_runtime_errors.emplace_back(ErrorType::Type, "Color expects 3 arguments.", node->token.line, node->token.column); return nullptr; }
            auto r_val = visit(node->arguments[0].get()); auto g_val = visit(node->arguments[1].get()); auto b_val = visit(node->arguments[2].get());
            if (!r_val || !g_val || !b_val || !std::holds_alternative<Integer>(r_val->data) || !std::holds_alternative<Integer>(g_val->data) || !std::holds_alternative<Integer>(b_val->data)) { m_runtime_errors.emplace_back(ErrorType::Type, "Color arguments must be integers.", node->token.line, node->token.column); return nullptr; }
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

    bool YiniRuntime::serialize(const std::string& filepath) const { return false; }
    bool YiniRuntime::deserialize(const std::string& filepath) { return false; }
}
