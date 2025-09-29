#ifndef YINI_EXCEPTION_HPP
#define YINI_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace YINI
{
class YiniException : public std::runtime_error
{
  public:
    YiniException(const std::string &message, int line, int column)
        : std::runtime_error(message), m_line(line), m_column(column)
    {
    }

    int getLine() const
    {
        return m_line;
    }
    int getColumn() const
    {
        return m_column;
    }

  private:
    int m_line;
    int m_column;
};
} // namespace YINI

#endif // YINI_EXCEPTION_HPP