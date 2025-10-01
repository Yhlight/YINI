#pragma once

#include "Ast.h"
#include <string>

namespace YINI
{
    class AstPrinter : public ExprVisitor
    {
    public:
        std::string print(const Expr& expr);
        std::any visit(const Literal& expr) override;
        std::any visit(const Unary& expr) override;
        std::any visit(const Binary& expr) override;
        std::any visit(const Grouping& expr) override;
        std::any visit(const Array& expr) override;

    private:
        std::string parenthesize(const std::string& name, const std::vector<std::reference_wrapper<const Expr>>& exprs);
    };
}