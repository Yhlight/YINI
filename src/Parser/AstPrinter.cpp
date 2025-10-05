#include "AstPrinter.h"

#include <functional>
#include <sstream>
#include <variant>
#include <vector>

namespace YINI {
std::string AstPrinter::print(const Expr& expr) {
    YiniValue result = expr.accept(*this);
    if (const auto* str = std::get_if<std::string>(&result.m_value)) {
        return *str;
    }
    return "[AstPrinter: Error printing expression]";
}

// Visitor for printing literal values
struct LiteralPrintVisitor {
    std::string operator()(std::monostate) const { return "nil"; }
    std::string operator()(bool value) const { return value ? "true" : "false"; }
    std::string operator()(double value) const {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    std::string operator()(const std::string& value) const { return value; }
    template <typename T>
    std::string operator()(const std::unique_ptr<T>&) const {
        return "unprintable_ptr";
    }
};

YiniValue AstPrinter::visit(const Literal& expr) { return std::visit(LiteralPrintVisitor{}, expr.value.m_value); }

YiniValue AstPrinter::visit(const Unary& expr) { return parenthesize(expr.op.lexeme, {expr.right.get()}); }

YiniValue AstPrinter::visit(const Binary& expr) {
    return parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
}

YiniValue AstPrinter::visit(const Grouping& expr) { return parenthesize("group", {expr.expression.get()}); }

YiniValue AstPrinter::visit(const Array& expr) {
    std::vector<const Expr*> exprs;
    for (const auto& element : expr.elements) {
        exprs.push_back(element.get());
    }
    return parenthesize("array", exprs);
}

YiniValue AstPrinter::visit(const Set& expr) {
    std::vector<const Expr*> exprs;
    for (const auto& element : expr.elements) {
        exprs.push_back(element.get());
    }
    return parenthesize("set", exprs);
}

YiniValue AstPrinter::visit(const Map& expr) {
    std::stringstream builder;
    builder << "(map";
    for (const auto& pair : expr.pairs) {
        builder << " (";
        builder << print(*pair.first);
        builder << " ";
        builder << print(*pair.second);
        builder << ")";
    }
    builder << ")";
    return builder.str();
}

YiniValue AstPrinter::visit(const Call& expr) {
    std::vector<const Expr*> exprs;
    exprs.push_back(expr.callee.get());
    for (const auto& argument : expr.arguments) {
        exprs.push_back(argument.get());
    }
    return parenthesize("call", exprs);
}

YiniValue AstPrinter::visit(const Variable& expr) { return expr.name.lexeme; }

YiniValue AstPrinter::visit(const EnvVariable& expr) {
    if (expr.default_value) {
        return parenthesize("${" + expr.name.lexeme + "}", {expr.default_value.get()});
    }
    return parenthesize("${" + expr.name.lexeme + "}", {});
}

YiniValue AstPrinter::visit(const XRef& expr) {
    return parenthesize("@{" + expr.section.lexeme + "." + expr.key.lexeme + "}", {});
}

std::string AstPrinter::parenthesize(const std::string& name, const std::vector<const Expr*>& exprs) {
    std::stringstream builder;
    builder << "(" << name;
    for (const auto* expr : exprs) {
        builder << " ";
        builder << print(*expr);
    }
    builder << ")";
    return builder.str();
}
}  // namespace YINI