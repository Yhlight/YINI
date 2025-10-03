/**
 * @file YiniException.h
 * @brief Defines a hierarchy of custom exception classes for the YINI library.
 * @ingroup Core
 */
#pragma once

#include <stdexcept>
#include <string>

namespace YINI
{
    /**
     * @class YiniException
     * @brief Base class for all custom exceptions in the YINI library.
     * @ingroup Core
     *
     * @details This exception is the parent for more specific error types and
     * provides detailed context, including the file path, line number, and column
     * number, to help users quickly identify the source of an error. It inherits
     * from `std::runtime_error`.
     *
     * @see ParsingError, RuntimeError
     */
    class YiniException : public std::runtime_error
    {
    public:
        /**
         * @brief Constructs a YiniException with detailed contextual information.
         * @param message The primary error message describing the issue.
         * @param line The line number in the source file where the error occurred.
         * @param column The column number where the error occurred. Defaults to 0 if not applicable.
         * @param filepath The path to the source file where the error occurred. Defaults to an empty string.
         */
        YiniException(const std::string& message, int line, int column = 0, const std::string& filepath = "")
            : std::runtime_error(message), m_line(line), m_column(column), m_filepath(filepath) {}

        /**
         * @brief Gets the line number where the error occurred.
         * @return The line number as an integer.
         */
        int line() const { return m_line; }

        /**
         * @brief Gets the column number where the error occurred.
         * @return The column number as an integer.
         */
        int column() const { return m_column; }

        /**
         * @brief Gets the path to the file where the error occurred.
         * @return A const reference to the file path string.
         */
        const std::string& filepath() const { return m_filepath; }

    protected:
        /// @brief The line number where the error was detected.
        int m_line;
        /// @brief The column number where the error was detected.
        int m_column;
        /// @brief The file path associated with the error.
        std::string m_filepath;
    };

    /**
     * @class ParsingError
     * @brief Exception thrown for errors that occur during the parsing phase.
     * @ingroup Core
     *
     * @details This exception indicates a syntax error or structural problem
     * discovered while reading and interpreting a YINI source file.
     * @see YiniException
     */
    class ParsingError : public YiniException
    {
    public:
        using YiniException::YiniException;
    };

    /**
     * @class RuntimeError
     * @brief Exception thrown for errors that occur during the runtime or interpretation phase.
     * @ingroup Core
     *
     * @details This exception is used for semantic errors, such as type mismatches,
     * undefined variable access, or failed arithmetic operations that occur after
     * a file has been successfully parsed.
     * @see YiniException
     */
    class RuntimeError : public YiniException
    {
    public:
        using YiniException::YiniException;
    };
}