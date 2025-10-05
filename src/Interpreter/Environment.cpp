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
        auto it = m_values.find(name.lexeme);
        if (it != m_values.end()) {
            return it->second.value;
        }
        throw RuntimeError("Undefined variable '" + name.lexeme + "'.", name.line, name.column, name.filepath);
    }

    std::optional<Token> Environment::get_definition_token(std::string_view name) const
    {
        auto it = m_values.find(name);
        if (it != m_values.end()) {
            return it->second.definition_token;
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