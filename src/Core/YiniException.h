#pragma once

#include <stdexcept>
#include <string>
#include <sstream>

namespace YINI
{
    class YiniException : public std::runtime_error
    {
    public:
        YiniException(const std::string& message, int line, int column)
            : std::runtime_error(message), m_filepath("<unknown>"), m_line(line), m_column(column)
        {
            build_full_message();
        }

        YiniException(const std::string& message, const std::string& filepath, int line, int column)
            : std::runtime_error(message), m_filepath(filepath), m_line(line), m_column(column)
        {
            build_full_message();
        }

        const char* what() const noexcept override
        {
            return m_full_message.c_str();
        }

        void set_filepath(const std::string& filepath)
        {
            m_filepath = filepath;
            build_full_message();
        }

        const std::string& filepath() const { return m_filepath; }
        int line() const { return m_line; }
        int column() const { return m_column; }

    private:
        void build_full_message()
        {
            std::stringstream ss;
            ss << m_filepath << ":" << m_line << ":" << m_column << ": " << std::runtime_error::what();
            m_full_message = ss.str();
        }

        std::string m_filepath;
        int m_line;
        int m_column;
        std::string m_full_message;
    };
}