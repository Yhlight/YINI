#pragma once

#include "Parser/Ast.h"
#include "Environment.h"
#include "Core/YiniValue.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace YINI
{
    struct ValueLocation
    {
        int line;
        int column;
    };

    class Interpreter : public ExprVisitor, public StmtVisitor
    {
    public:
        void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);
        void clear();
        std::string stringify(const YiniValue& value);
        std::vector<std::string> get_macro_names() const;

        std::map<std::string, std::map<std::string, YiniValue>> resolved_sections;
        std::map<std::string, std::map<std::string, ValueLocation>> value_locations;

        // Statement visitor methods
        void visit(const KeyValue& stmt) override;
        void visit(const Section& stmt) override;
        void visit(const Register& stmt) override;
        void visit(const Define& stmt) override;
        void visit(const Include& stmt) override;
        void visit(const Schema& stmt) override;

        // Expression visitor methods
        YiniValue visit(const Literal& expr) override;
        YiniValue visit(const Unary& expr) override;
        YiniValue visit(const Binary& expr) override;
        YiniValue visit(const Grouping& expr) override;
        YiniValue visit(const Array& expr) override;
        YiniValue visit(const Set& expr) override;
        YiniValue visit(const Map& expr) override;
        YiniValue visit(const Call& expr) override;
        YiniValue visit(const Variable& expr) override;
        YiniValue visit(const EnvVariable& expr) override;
        YiniValue visit(const XRef& expr) override;

    private:
        YiniValue evaluate(const Expr& expr);
        void execute(const Stmt& stmt);
        void build_expression_map(const Section* section);

        Environment m_globals;
        std::map<std::string, const Section*> m_sections;
        std::set<std::string> m_resolved;
        std::set<std::string> m_resolving; // For cycle detection

        // New members for on-demand resolution
        std::map<std::string, std::map<std::string, const Expr*>> m_expression_map;
        std::map<std::string, std::map<std::string, const KeyValue*>> m_kv_map; // For error reporting
        std::set<std::string> m_currently_resolving_values; // For cycle detection
        std::string m_current_section_name;
    };
}