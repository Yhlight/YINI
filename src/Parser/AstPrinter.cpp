#include "AstPrinter.h"
#include <sstream>
#include <vector>
#include <functional>

namespace YINI
{
    std::string AstPrinter::print(const Expr& expr)
    {
        return std::any_cast<std::string>(expr.accept(*this));
    }

    std::any AstPrinter::visit(const Literal& expr)
    {
        if (expr.value.type() == typeid(std::string))
        {
            return std::any_cast<std::string>(expr.value);
        }
        if (expr.value.type() == typeid(double))
        {
            std::stringstream ss;
            ss << std::any_cast<double>(expr.value);
            return ss.str();
        }
        if (expr.value.type() == typeid(bool))
        {
            return std::string(std::any_cast<bool>(expr.value) ? "true" : "false");
        }
        return std::string("nil");
    }

    std::any AstPrinter::visit(const Unary& expr)
    {
        return parenthesize(expr.op.lexeme, {std::cref(*expr.right)});
    }

    std::any AstPrinter::visit(const Binary& expr)
    {
        return parenthesize(expr.op.lexeme, {std::cref(*expr.left), std::cref(*expr.right)});
    }

    std::any AstPrinter::visit(const Grouping& expr)
    {
        return parenthesize("group", {std::cref(*expr.expression)});
    }

    std::any AstPrinter::visit(const Array& expr)
    {
        std::vector<std::reference_wrapper<const Expr>> exprs;
        for (const auto& element : expr.elements)
        {
            exprs.push_back(std::cref(*element));
        }
        return parenthesize("array", exprs);
    }

    std::any AstPrinter::visit(const Set& expr)
    {
        std::vector<std::reference_wrapper<const Expr>> exprs;
        for (const auto& element : expr.elements)
        {
            exprs.push_back(std::cref(*element));
        }
        return parenthesize("set", exprs);
    }

    std::any AstPrinter::visit(const Map& expr)
    {
        std::stringstream builder;
        builder << "(map";
        for (const auto& pair : expr.pairs)
        {
            builder << " (";
            builder << std::any_cast<std::string>(pair.first->accept(*this));
            builder << " ";
            builder << std::any_cast<std::string>(pair.second->accept(*this));
            builder << ")";
        }
        builder << ")";
        return builder.str();
    }

    std::any AstPrinter::visit(const Call& expr)
    {
        std::vector<std::reference_wrapper<const Expr>> exprs;
        exprs.push_back(std::cref(*expr.callee));
        for (const auto& argument : expr.arguments)
        {
            exprs.push_back(std::cref(*argument));
        }
        return parenthesize("call", exprs);
    }

    std::any AstPrinter::visit(const Variable& expr)
    {
        return expr.name.lexeme;
    }

    std::string AstPrinter::parenthesize(const std::string& name, const std::vector<std::reference_wrapper<const Expr>>& exprs)
    {
        std::stringstream builder;
        builder << "(" << name;
        for (const auto& expr_ref : exprs)
        {
            const Expr& expr = expr_ref.get();
            builder << " ";
            builder << std::any_cast<std::string>(expr.accept(*this));
        }
        builder << ")";
        return builder.str();
    }
}