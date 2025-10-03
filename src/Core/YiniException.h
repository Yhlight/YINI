/**
 * @file YiniException.h
 * @brief Defines the YiniException class for custom error handling.
 */
#pragma once

#include <stdexcept>
#include <string>

namespace YINI
{
    /**
     * @class YiniException
     * @brief Custom exception class for the YINI library.
     *
     * This exception is thrown for parsing, interpretation, and other runtime
     * errors. It contains detailed context, including the file path, line number,
     * and column number, to help users quickly identify the source of an error.
     */
    class YiniException : public std::runtime_error
    {
    public:
        /**
         * @brief Constructs a YiniException.
         * @param message The error message.
         * @param line The line number where the error occurred.
         * @param column The column number where the error occurred (optional).
         * @param filepath The path to the file where the error occurred (optional).
         */
        YiniException(const std::string& message, int line, int column = 0, const std::string& filepath = "")
            : std::runtime_error(message), m_line(line), m_column(column), m_filepath(filepath) {}

        /**
         * @brief Gets the line number of the error.
         * @return The line number.
         */
        int line() const { return m_line; }

        /**
         * @brief Gets the column number of the error.
         * @return The column number.
         */
        int column() const { return m_column; }

        /**
         * @brief Gets the file path where the error occurred.
         * @return A const reference to the file path string.
         */
        const std::string& filepath() const { return m_filepath; }

    protected:
        int m_line;
        int m_column;
        std::string m_filepath;
    };

    /**
     * @class ParsingError
     * @brief Exception for errors that occur during the parsing phase.
     */
    class ParsingError : public YiniException
    {
    public:
        using YiniException::YiniException;
    };

    /**
     * @class RuntimeError
     * @brief Exception for errors that occur during the runtime/interpretation phase.
     */
    class RuntimeError : public YiniException
    {
    public:
        using YiniException::YiniException;
    };
}