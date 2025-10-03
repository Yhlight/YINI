#pragma once

#include "Ast.h"
#include <string>

namespace YINI
{
    class AstPrinter : public ExprVisitor
    {
    public:
        std::string print(const Expr& expr);
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

    private:
        std::string parenthesize(const std::string& name, const std::vector<const Expr*>& exprs);
    };
}