#include "Environment.h"
#include "Core/YiniException.h"
#include <stdexcept>

namespace YINI
{
    void Environment::define(const std::string& name, std::any value)
    {
        m_values[name] = value;
    }

    std::any Environment::get(const Token& name)
    {
        if (m_values.count(name.lexeme))
        {
            return m_values.at(name.lexeme);
        }

        throw YiniException("Undefined variable '" + name.lexeme + "'.", name.line);
    }
}