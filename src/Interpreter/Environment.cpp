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
}