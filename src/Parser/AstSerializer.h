#pragma once

#include "Ast.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace YINI
{
    class AstSerializer : public ExprVisitor, public StmtVisitor
    {
    public:
        json serialize(const std::vector<std::unique_ptr<Stmt>>& statements);

        void visit(const LiteralExpr& expr) override;
        void visit(const BinaryExpr& expr) override;
        void visit(const GroupingExpr& expr) override;
        void visit(const ArrayExpr& expr) override;
        void visit(const CallExpr& expr) override;
        void visit(const KeyValuePairExpr& expr) override;
        void visit(const MapExpr& expr) override;
        void visit(const DynaExpr& expr) override;

        void visit(const SectionStmt& stmt) override;
        void visit(const KeyValueStmt& stmt) override;
        void visit(const RegisterStmt& stmt) override;

    private:
        json m_currentJson;
    };
}