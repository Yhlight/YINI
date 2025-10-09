#ifndef PARSING_EXCEPTION_H
#define PARSING_EXCEPTION_H

#include <stdexcept>
#include <string>

class ParsingException : public std::runtime_error {
public:
    ParsingException(const std::string& message, size_t line, size_t column)
        : std::runtime_error(message), line(line), column(column) {}

    size_t getLine() const { return line; }
    size_t getColumn() const { return column; }

private:
    size_t line;
    size_t column;
};

#endif // PARSING_EXCEPTION_H