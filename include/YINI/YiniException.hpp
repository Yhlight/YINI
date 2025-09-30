/**
 * @file YiniException.hpp
 * @brief Defines the custom exception type for the YINI library.
 */

#ifndef YINI_EXCEPTION_HPP
#define YINI_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace YINI
{
/**
 * @class YiniException
 * @brief A custom exception for parsing and runtime errors within the YINI
 * library.
 * @details This exception extends `std::runtime_error` to include line and
 * column numbers, providing precise location information for syntax errors.
 */
class YiniException : public std::runtime_error
{
public:
  /**
   * @brief Constructs a YiniException.
   * @param message The error message.
   * @param line The line number where the error occurred.
   * @param column The column number where the error occurred.
   */
  YiniException(const std::string &message, int line, int column)
      : std::runtime_error(message), line(line), column(column)
  {
  }

  /**
   * @brief Gets the line number of the error.
   * @return The line number.
   */
  int getLine() const { return line; }

  /**
   * @brief Gets the column number of the error.
   * @return The column number.
   */
  int getColumn() const { return column; }

private:
  int line;   ///< The line number where the error occurred.
  int column; ///< The column number where the error occurred.
};
} // namespace YINI

#endif // YINI_EXCEPTION_HPP