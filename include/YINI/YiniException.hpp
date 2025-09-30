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

/**
 * @class ParsingException
 * @brief An exception for syntax errors during parsing.
 */
class ParsingException : public YiniException
{
public:
  using YiniException::YiniException; // Inherit constructor
};

/**
 * @class IOException
 * @brief An exception for file-related errors (e.g., file not found).
 */
class IOException : public YiniException
{
public:
  using YiniException::YiniException; // Inherit constructor
};

/**
 * @class LogicException
 * @brief An exception for logical errors (e.g., circular inheritance).
 */
class LogicException : public YiniException
{
public:
  using YiniException::YiniException; // Inherit constructor
};

} // namespace YINI

#endif // YINI_EXCEPTION_HPP