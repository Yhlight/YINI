#include "AstSerializer.h"

namespace YINI
{
    json AstSerializer::serialize(const std::vector<std::unique_ptr<Stmt>>& statements)
    {
        json root = json::array();
        for (const auto& stmt : statements)
        {
            stmt->accept(*this);
            root.push_back(m_currentJson);
        }
        return root;
    }

    void AstSerializer::visit(const LiteralExpr& expr)
    {
        json node;
        node["type"] = "Literal";
        if (std::holds_alternative<std::string>(expr.token.literal))
        {
            node["value"] = std::get<std::string>(expr.token.literal);
        }
        else if (std::holds_alternative<long long>(expr.token.literal))
        {
            node["value"] = std::get<long long>(expr.token.literal);
        }
        else if (std::holds_alternative<double>(expr.token.literal))
        {
            node["value"] = std::get<double>(expr.token.literal);
        }
        else
        {
            node["value"] = expr.token.lexeme;
        }
        m_currentJson = node;
    }

    void AstSerializer::visit(const BinaryExpr& expr)
    {
        json node;
        node["type"] = "Binary";
        node["operator"] = expr.op.lexeme;
        expr.left->accept(*this);
        node["left"] = m_currentJson;
        expr.right->accept(*this);
        node["right"] = m_currentJson;
        m_currentJson = node;
    }

    void AstSerializer::visit(const GroupingExpr& expr)
    {
        json node;
        node["type"] = "Grouping";
        expr.expression->accept(*this);
        node["expression"] = m_currentJson;
        m_currentJson = node;
    }

    void AstSerializer::visit(const ArrayExpr& expr)
    {
        json node;
        node["type"] = "Array";
        node["elements"] = json::array();
        for (const auto& element : expr.elements)
        {
            element->accept(*this);
            node["elements"].push_back(m_currentJson);
        }
        m_currentJson = node;
    }

    void AstSerializer::visit(const CallExpr& expr)
    {
        json node;
        node["type"] = "Call";
        expr.callee->accept(*this);
        node["callee"] = m_currentJson;
        node["arguments"] = json::array();
        for (const auto& arg : expr.arguments)
        {
            arg->accept(*this);
            node["arguments"].push_back(m_currentJson);
        }
        m_currentJson = node;
    }

    void AstSerializer::visit(const KeyValuePairExpr& expr)
    {
        json node;
        node["type"] = "KeyValuePair";
        node["key"] = expr.key.lexeme;
        expr.value->accept(*this);
        node["value"] = m_currentJson;
        m_currentJson = node;
    }

    void AstSerializer::visit(const MapExpr& expr)
    {
        json node;
        node["type"] = "Map";
        node["pairs"] = json::array();
        for (const auto& pair : expr.pairs)
        {
            pair->accept(*this);
            node["pairs"].push_back(m_currentJson);
        }
        m_currentJson = node;
    }

    void AstSerializer::visit(const DynaExpr& expr)
    {
        json node;
        node["type"] = "Dyna";
        expr.expression->accept(*this);
        node["expression"] = m_currentJson;
        m_currentJson = node;
    }

    void AstSerializer::visit(const SectionStmt& stmt)
    {
        json node;
        node["type"] = "Section";
        node["name"] = stmt.name.lexeme;
        node["inheritance"] = json::array();
        for (const auto& token : stmt.inheritance)
        {
            node["inheritance"].push_back(token.lexeme);
        }
        node["statements"] = json::array();
        for (const auto& statement : stmt.statements)
        {
            statement->accept(*this);
            node["statements"].push_back(m_currentJson);
        }
        m_currentJson = node;
    }

    void AstSerializer::visit(const KeyValueStmt& stmt)
    {
        json node;
        node["type"] = "KeyValue";
        node["key"] = stmt.key.lexeme;
        stmt.value->accept(*this);
        node["value"] = m_currentJson;
        m_currentJson = node;
    }

    void AstSerializer::visit(const RegisterStmt& stmt)
    {
        json node;
        node["type"] = "Register";
        node["operator"] = stmt.op.lexeme;
        stmt.value->accept(*this);
        node["value"] = m_currentJson;
        m_currentJson = node;
    }
}