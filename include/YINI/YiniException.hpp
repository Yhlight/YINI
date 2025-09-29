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
            : std::runtime_error(message), line_num(line), column_num(column) {}

        int getLine() const { return line_num; }
        int getColumn() const { return column_num; }

    private:
        int line_num;
        int column_num;
    };
}

#endif // YINI_EXCEPTION_HPP