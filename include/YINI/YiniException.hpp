#ifndef YINI_EXCEPTION_HPP
#define YINI_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <vector>

namespace YINI
{
    /**
     * @brief Represents a single syntax error found by the parser.
     */
    struct YiniSyntaxError {
        std::string message;
        int line;
        int column;
    };

    /**
     * @brief An exception that aggregates all syntax errors found during a parse.
     */
    class YiniParsingException : public std::runtime_error
    {
    public:
        YiniParsingException(const std::vector<YiniSyntaxError>& errors)
            : std::runtime_error("YINI parsing failed with one or more errors."), m_errors(errors) {}

        const std::vector<YiniSyntaxError>& getErrors() const { return m_errors; }

    private:
        std::vector<YiniSyntaxError> m_errors;
    };
}

#endif // YINI_EXCEPTION_HPP