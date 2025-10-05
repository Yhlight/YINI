#include "Environment.h"
#include "Core/YiniException.h"

namespace YINI
{
    void Environment::define(const Token& name_token, YiniValue value)
    {
        m_values[name_token.lexeme] = { std::move(value), name_token };
    }

    YiniValue Environment::get(const Token& name) const
    {
        if (m_values.count(name.lexeme))
        {
            return m_values.at(name.lexeme).value;
        }
        throw RuntimeError("Undefined variable '" + name.lexeme + "'.", name.line, name.column, name.filepath);
    }

    std::optional<Token> Environment::get_definition_token(const std::string& name) const
    {
        if (m_values.count(name))
        {
            return m_values.at(name).definition_token;
        }
        return std::nullopt;
    }

    void Environment::clear()
    {
        m_values.clear();
    }

    std::vector<std::string> Environment::get_all_keys() const
    {
        std::vector<std::string> keys;
        for (const auto& pair : m_values)
        {
            keys.push_back(pair.first);
        }
        return keys;
    }
}