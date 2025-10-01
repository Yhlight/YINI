#include "Environment.h"
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

        throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
    }
}