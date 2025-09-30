#ifndef YINI_EXCEPTION_HPP
#define YINI_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace YINI
{
    class YiniException : public std::runtime_error
    {
    public:
        YiniException(const std::string& message, int line, int column)
            : std::runtime_error(message), line(line), column(column) {}

        int getLine() const { return line; }
        int getColumn() const { return column; }

    private:
        int line;
        int column;
    };
}

#endif // YINI_EXCEPTION_HPP