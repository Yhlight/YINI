#include "Resolver.h"
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace YINI
{

// Helper function to convert YiniVariant to std::any for YmetaManager
static std::any to_any(const YiniVariant &variant)
{
    if (std::holds_alternative<std::monostate>(variant))
    {
        return {};
    }
    else if (std::holds_alternative<int64_t>(variant))
    {
        return static_cast<double>(std::get<int64_t>(variant)); // Store as double for consistency
    }
    else if (std::holds_alternative<double>(variant))
    {
        return std::get<double>(variant);
    }
    else if (std::holds_alternative<bool>(variant))
    {
        return std::get<bool>(variant);
    }
    else if (std::holds_alternative<std::string>(variant))
    {
        return std::get<std::string>(variant);
    }
    else if (std::holds_alternative<std::unique_ptr<YiniArray>>(variant))
    {
        std::vector<std::any> vec;
        const auto &arr = *std::get<std::unique_ptr<YiniArray>>(variant);
        for (const auto &item : arr)
        {
            vec.push_back(to_any(item));
        }
        return vec;
    }
    // Ignoring Color and Coord for Ymeta for now
    return {};
}

Resolver::Resolver(const std::vector<std::unique_ptr<AST::Stmt>> &statements, YmetaManager &ymeta_manager)
    : m_statements(statements), m_ymeta_manager(ymeta_manager)
{
}

std::map<std::string, YiniVariant> Resolver::resolve()
{
    // First pass: collect all section definitions and macros from the root and included files.
    collect_declarations(m_statements);

    // Second pass: resolve each section, handling inheritance.
    for (const auto &pair : m_section_nodes)
    {
        resolve_section(pair.first);
    }

    // Third pass: flatten the resolved per-section data into the final config map.
    for (const auto &section_pair : m_resolved_sections_data)
    {
        const std::string &section_name = section_pair.first;
        const auto &section_data = section_pair.second;
        for (const auto &value_pair : section_data)
        {
            const std::string &key = value_pair.first;
            const YiniVariant &value = value_pair.second;
            m_resolved_config[section_name + "." + key] = value;
        }
    }

    return m_resolved_config;
}

void Resolver::collect_declarations(const std::vector<std::unique_ptr<AST::Stmt>> &statements)
{
    for (const auto &stmt : statements)
    {
        if (auto *define_stmt = dynamic_cast<AST::DefineSectionStmt *>(stmt.get()))
        {
            for (const auto &definition : define_stmt->definitions)
            {
                m_macros[definition->key.lexeme] = definition->value.get();
            }
        }
        else if (auto *section_stmt = dynamic_cast<AST::SectionStmt *>(stmt.get()))
        {
            const std::string &name = section_stmt->name.lexeme;
            if (m_section_nodes.find(name) == m_section_nodes.end())
            {
                m_section_nodes[name] = section_stmt;
            }
            else
            {
                // Merge statements if the section is defined multiple times.
                m_section_nodes[name]->statements.insert(m_section_nodes[name]->statements.end(),
                                                         std::make_move_iterator(section_stmt->statements.begin()),
                                                         std::make_move_iterator(section_stmt->statements.end()));
            }
        }
        else if (auto *include_stmt = dynamic_cast<AST::IncludeStmt *>(stmt.get()))
        {
            visitIncludeStmt(include_stmt, true); // Pass true to indicate collection mode.
        }
    }
}

std::map<std::string, YiniVariant> Resolver::resolve_section(const std::string &section_name)
{
    // If already resolved, return the cached data.
    if (m_resolved_sections_data.count(section_name))
    {
        return m_resolved_sections_data[section_name];
    }

    // Check for circular dependencies.
    if (m_resolving_stack.count(section_name))
    {
        throw std::runtime_error("Circular inheritance detected involving section: " + section_name);
    }

    if (m_section_nodes.find(section_name) == m_section_nodes.end())
    {
        throw std::runtime_error("Reference to undefined section: " + section_name);
    }

    m_resolving_stack.insert(section_name);

    m_current_section_name = section_name; // Set context for DynaExpr
    AST::SectionStmt *section_stmt = m_section_nodes[section_name];
    std::map<std::string, YiniVariant> section_data;

    // Resolve and merge parent sections first.
    for (const auto &parent_token : section_stmt->parent_sections)
    {
        const std::string &parent_name = parent_token.lexeme;
        auto parent_data = resolve_section(parent_name);
        for (const auto &pair : parent_data)
        {
            section_data[pair.first] = pair.second;
        }
    }

    // Resolve this section's own key-value pairs, overriding any from parents.
    m_current_section_data = &section_data;
    for (const auto &statement : section_stmt->statements)
    {
        statement->accept(this);
    }
    m_current_section_data = nullptr;

    m_resolving_stack.erase(section_name);
    m_resolved_sections_data[section_name] = section_data;
    return section_data;
}

void Resolver::visitDefineSectionStmt(AST::DefineSectionStmt *stmt)
{
    // This is handled in the collection pass.
}

void Resolver::visitSectionStmt(AST::SectionStmt *stmt)
{
    // This is handled by the resolve_section method.
}

void Resolver::visitIncludeStmt(AST::IncludeStmt *stmt, bool collection_mode)
{
    for (const auto &path_expr : stmt->paths)
    {
        YiniVariant path_variant = path_expr->accept(this);
        if (!std::holds_alternative<std::string>(path_variant))
        {
            throw std::runtime_error("Include path must be a string.");
        }
        std::string path = std::get<std::string>(path_variant);

        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open included file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        Lexer lexer(buffer.str());
        auto tokens = lexer.scan_tokens();
        Parser parser(tokens);
        auto included_ast = parser.parse();

        if (collection_mode)
        {
            collect_declarations(included_ast);
            m_included_asts.push_back(std::move(included_ast));
        }
    }
}

void Resolver::visitKeyValueStmt(AST::KeyValueStmt *stmt)
{
    if (!m_current_section_data)
        return;

    std::string key = stmt->key.lexeme;
    std::string full_key = m_current_section_name + "." + key;

    if (auto *dyna_expr = dynamic_cast<AST::DynaExpr *>(stmt->value.get()))
    {
        if (m_ymeta_manager.has_value(full_key))
        {
            // YmetaManager returns std::any, we need to convert it to YiniVariant
            std::any val = m_ymeta_manager.get_value(full_key);
            if (val.type() == typeid(double))
            {
                (*m_current_section_data)[key] = std::any_cast<double>(val);
            }
            else if (val.type() == typeid(bool))
            {
                (*m_current_section_data)[key] = std::any_cast<bool>(val);
            }
            else if (val.type() == typeid(std::string))
            {
                (*m_current_section_data)[key] = std::any_cast<std::string>(val);
            }
        }
        else
        {
            YiniVariant value = dyna_expr->expression->accept(this);
            m_ymeta_manager.set_value(full_key, to_any(value));
            (*m_current_section_data)[key] = value;
        }
    }
    else
    {
        (*m_current_section_data)[key] = stmt->value->accept(this);
    }
}

void Resolver::visitQuickRegStmt(AST::QuickRegStmt *stmt)
{
    if (!m_current_section_data)
    {
        throw std::runtime_error("Quick registration '+=' can only be used inside a section.");
    }

    int max_index = -1;
    for (const auto &pair : *m_current_section_data)
    {
        try
        {
            int current_key = std::stoi(pair.first);
            if (current_key > max_index)
            {
                max_index = current_key;
            }
        }
        catch (const std::invalid_argument &)
        {
            // Ignore keys that are not integers
        }
    }

    std::string key = std::to_string(max_index + 1);
    (*m_current_section_data)[key] = stmt->value->accept(this);
}

void Resolver::visitSchemaRuleStmt(AST::SchemaRuleStmt *stmt)
{
}
void Resolver::visitSchemaSectionStmt(AST::SchemaSectionStmt *stmt)
{
}
void Resolver::visitSchemaStmt(AST::SchemaStmt *stmt)
{
}

YiniVariant Resolver::visitLiteralExpr(AST::LiteralExpr *expr)
{
    if (std::holds_alternative<std::string>(expr->value.literal))
    {
        return std::get<std::string>(expr->value.literal);
    }
    else if (std::holds_alternative<double>(expr->value.literal))
    {
        // Try to store as int64_t if it's a whole number
        double val = std::get<double>(expr->value.literal);
        if (std::fmod(val, 1.0) == 0.0)
        {
            return static_cast<int64_t>(val);
        }
        return val;
    }
    return std::monostate{};
}

YiniVariant Resolver::visitBoolExpr(AST::BoolExpr *expr)
{
    return expr->value;
}

YiniVariant Resolver::visitArrayExpr(AST::ArrayExpr *expr)
{
    auto arr = std::make_unique<YiniArray>();
    if (expr->elements.empty())
    {
        return arr;
    }

    for (const auto &element : expr->elements)
    {
        arr->push_back(element->accept(this));
    }

    // Verify that all elements in the array have the same type.
    auto first_element_type_index = arr->front().index();
    for (size_t i = 1; i < arr->size(); ++i)
    {
        if (arr->at(i).index() != first_element_type_index)
        {
            throw std::runtime_error("Array contains mixed types. All elements in an array must have the same type.");
        }
    }

    return arr;
}

YiniVariant Resolver::visitSetExpr(AST::SetExpr *expr)
{
    // Semantically, we resolve sets to arrays for now
    auto arr = std::make_unique<YiniArray>();
    for (const auto &element : expr->elements)
    {
        arr->push_back(element->accept(this));
    }
    return arr;
}

YiniVariant Resolver::visitMapExpr(AST::MapExpr *expr)
{
    YiniMap map;
    for (const auto &element : expr->elements)
    {
        map[element.first.lexeme] = element.second->accept(this);
    }
    return map;
}

YiniVariant Resolver::visitStructExpr(AST::StructExpr *expr)
{
    auto value_variant = expr->value->accept(this);
    auto yini_struct = std::make_unique<YiniVariant>(std::move(value_variant));
    return YiniStruct(expr->key.lexeme, std::move(yini_struct));
}

YiniVariant Resolver::visitColorExpr(AST::ColorExpr *expr)
{
    ResolvedColor color;
    color.r = expr->r;
    color.g = expr->g;
    color.b = expr->b;
    return color;
}

YiniVariant Resolver::visitCoordExpr(AST::CoordExpr *expr)
{
    ResolvedCoord coord;
    YiniVariant x = expr->x->accept(this);
    YiniVariant y = expr->y->accept(this);

    if (std::holds_alternative<double>(x))
        coord.x = std::get<double>(x);
    if (std::holds_alternative<int64_t>(x))
        coord.x = std::get<int64_t>(x);
    if (std::holds_alternative<double>(y))
        coord.y = std::get<double>(y);
    if (std::holds_alternative<int64_t>(y))
        coord.y = std::get<int64_t>(y);

    if (expr->z)
    {
        coord.has_z = true;
        YiniVariant z = expr->z->accept(this);
        if (std::holds_alternative<double>(z))
            coord.z = std::get<double>(z);
        if (std::holds_alternative<int64_t>(z))
            coord.z = std::get<int64_t>(z);
    }
    return coord;
}

YiniVariant Resolver::visitMacroExpr(AST::MacroExpr *expr)
{
    if (m_macros.find(expr->name.lexeme) == m_macros.end())
    {
        throw std::runtime_error("Error at line " + std::to_string(expr->name.line) + ", column " +
                                 std::to_string(expr->name.column) + ": Undefined macro: " + expr->name.lexeme);
    }
    return m_macros[expr->name.lexeme]->accept(this);
}

YiniVariant Resolver::visitBinaryExpr(AST::BinaryExpr *expr)
{
    YiniVariant left = expr->left->accept(this);
    YiniVariant right = expr->right->accept(this);

    double left_val = 0.0;
    if (std::holds_alternative<double>(left))
        left_val = std::get<double>(left);
    if (std::holds_alternative<int64_t>(left))
        left_val = std::get<int64_t>(left);

    double right_val = 0.0;
    if (std::holds_alternative<double>(right))
        right_val = std::get<double>(right);
    if (std::holds_alternative<int64_t>(right))
        right_val = std::get<int64_t>(right);

    switch (expr->op.type)
    {
    case TokenType::PLUS:
        return left_val + right_val;
    case TokenType::MINUS:
        return left_val - right_val;
    case TokenType::STAR:
        return left_val * right_val;
    case TokenType::SLASH:
        if (right_val == 0)
            throw std::runtime_error("Division by zero.");
        return left_val / right_val;
    case TokenType::PERCENT:
        if (right_val == 0)
            throw std::runtime_error("Division by zero.");
        return fmod(left_val, right_val);
    default:
        break;
    }

    throw std::runtime_error("Operands must be numbers for arithmetic operations.");
}

YiniVariant Resolver::visitUnaryExpr(AST::UnaryExpr *expr)
{
    YiniVariant right = expr->right->accept(this);

    if (expr->op.type == TokenType::MINUS)
    {
        if (std::holds_alternative<double>(right))
            return -std::get<double>(right);
        if (std::holds_alternative<int64_t>(right))
            return -std::get<int64_t>(right);
        else
            throw std::runtime_error("Operand must be a number for unary minus.");
    }
    return {};
}

YiniVariant Resolver::visitGroupingExpr(AST::GroupingExpr *expr)
{
    return expr->expression->accept(this);
}

YiniVariant Resolver::visitCrossSectionRefExpr(AST::CrossSectionRefExpr *expr)
{
    const std::string &section_name = expr->section.lexeme;
    const std::string &key = expr->key.lexeme;

    resolve_section(section_name);

    const auto &section_data = m_resolved_sections_data.at(section_name);
    if (section_data.find(key) == section_data.end())
    {
        throw std::runtime_error("Error: Undefined key '" + key + "' in section '" + section_name + "'.");
    }
    return section_data.at(key);
}

YiniVariant Resolver::visitEnvVarRefExpr(AST::EnvVarRefExpr *expr)
{
    const char *value = std::getenv(expr->name.lexeme.c_str());
    if (value == nullptr)
        return std::string("");
    return std::string(value);
}

YiniVariant Resolver::visitDynaExpr(AST::DynaExpr *expr)
{
    return expr->expression->accept(this);
}

YiniVariant Resolver::visitPathExpr(AST::PathExpr *expr)
{
    return expr->path;
}

YiniVariant Resolver::visitListExpr(AST::ListExpr *expr)
{
    // Semantically, we resolve lists to arrays
    auto arr = std::make_unique<YiniArray>();
    for (const auto &element : expr->elements)
    {
        arr->push_back(element->accept(this));
    }
    return arr;
}

} // namespace YINI
