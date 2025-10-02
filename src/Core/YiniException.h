#pragma once

#include <stdexcept>
#include <string>

namespace YINI
{
    class YiniException : public std::runtime_error
    {
    public:
        YiniException(const std::string& message, int line, int column = 0, const std::string& filepath = "")
            : std::runtime_error(message), m_line(line), m_column(column), m_filepath(filepath) {}

        int line() const { return m_line; }
        int column() const { return m_column; }
        const std::string& filepath() const { return m_filepath; }

    private:
        int m_line;
        int m_column;
        std::string m_filepath;
    };
}