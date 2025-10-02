#pragma once

#include "Parser/Ast.h"
#include "Environment.h"
#include <vector>
#include <any>
#include <map>
#include <string>
#include <set>

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
        std::string stringify(const std::any& value);

        std::map<std::string, std::map<std::string, std::any>> resolved_sections;
        std::map<std::string, std::map<std::string, ValueLocation>> value_locations;

        // Statement visitor methods
        void visit(const KeyValue& stmt) override;
        void visit(const Section& stmt) override;
        void visit(const Register& stmt) override;
        void visit(const Define& stmt) override;
        void visit(const Include& stmt) override;

        // Expression visitor methods
        std::any visit(const Literal& expr) override;
        std::any visit(const Unary& expr) override;
        std::any visit(const Binary& expr) override;
        std::any visit(const Grouping& expr) override;
        std::any visit(const Array& expr) override;
        std::any visit(const Set& expr) override;
        std::any visit(const Map& expr) override;
        std::any visit(const Call& expr) override;
        std::any visit(const Variable& expr) override;
        const Environment& get_globals() const { return m_globals; }

    private:
        std::any evaluate(const Expr& expr);
        void execute(const Stmt& stmt);
        void resolve_section(const Section* section);

        Environment m_globals;

    private:
        std::map<std::string, const Section*> m_sections;
        std::set<std::string> m_resolved;
        std::set<std::string> m_resolving; // For cycle detection
        std::string m_current_section_name;
    };
}