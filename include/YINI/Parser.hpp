/**
 * @file Parser.hpp
 * @brief Defines the syntax parser for the YINI language.
 */

#ifndef YINI_PARSER_HPP
#define YINI_PARSER_HPP

#include "Lexer.hpp"
#include "YiniData.hpp"
#include <memory>
#include <string>

namespace YINI
{
/**
 * @class Parser
 * @brief Consumes tokens from a Lexer to build a YiniDocument.
 * @details This is a recursive descent parser that processes a stream of tokens
 *          and constructs an in-memory representation of a YINI file according
 *          to the language's grammar. It handles all syntactic structures like
 *          sections, key-value pairs, includes, and complex value types.
 */
class Parser
{
public:
  /**
   * @brief Constructs a new Parser.
   * @param content The raw string content of the YINI file to parse.
   * @param document A reference to the YiniDocument to populate.
   * @param basePath The base directory path used to resolve file includes.
   */
  Parser(const std::string &content, YiniDocument &document,
         const std::string &basePath = ".");

  /**
   * @brief Begins the parsing process.
   * @throws YiniException on syntax errors.
   */
  void parse();

private:
  void parseSection();
  void parseKeyValuePair(YiniSection &section);
  void parseQuickRegistration(YiniSection &section);
  YiniValue parseValue();
  std::unique_ptr<YiniArray> parseArray();
  std::unique_ptr<YiniArray> parseArrayFromFunction();
  std::unique_ptr<YiniList> parseList();
  std::unique_ptr<YiniSet> parseSet();
  std::unique_ptr<YiniMap> parseMap();
  YiniValue parseExpression();
  YiniValue parseTerm();
  YiniValue parseFactor();
  std::unique_ptr<YiniCoord> parseCoord();
  std::unique_ptr<YiniColor> parseColor();
  std::unique_ptr<YiniPath> parsePath();

  void nextToken();

  Lexer lexer;            ///< The lexical analyzer providing the token stream.
  Token currentToken;     ///< The current token being processed.
  YiniDocument &document; ///< The document object model to build.
  std::string basePath;   ///< The base path for resolving includes.
};
} // namespace YINI

#endif // YINI_PARSER_HPP