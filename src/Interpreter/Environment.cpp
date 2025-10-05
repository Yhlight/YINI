#include "Environment.h"
#include "Core/YiniException.h"

namespace YINI
{
    void Environment::define(const std::string& name, YiniValue value)
    {
        m_values[name] = std::move(value);
    }

    YiniValue Environment::get(const Token& name) const
    {
        if (m_values.count(name.lexeme))
        {
            return m_values.at(name.lexeme);
        }
        throw RuntimeError("Undefined variable '" + name.lexeme + "'.", name.line, name.column, name.filepath);
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