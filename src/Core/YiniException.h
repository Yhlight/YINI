#pragma once

#include <stdexcept>
#include <string>

namespace YINI
{
    class YiniException : public std::runtime_error
    {
    public:
        YiniException(const std::string& message, int line)
            : std::runtime_error(message), m_line(line) {}

        int line() const { return m_line; }

    private:
        int m_line;
    };
}