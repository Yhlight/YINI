#ifndef YINI_PARSER_HPP
#define YINI_PARSER_HPP

#include "Lexer.hpp"
#include "YiniData.hpp"
#include "YiniException.hpp"
#include <memory>
#include <string>
#include <vector>

namespace YINI
{
class Parser
{
public:
  Parser(const std::string &content, YiniDocument &document,
         const std::string &basePath = ".");
  void parse();
  bool hadError() const;
  const std::vector<YiniSyntaxError>& getErrors() const;

private:
  void reportError(const std::string& message);
  void synchronize();
  void parseSection();
  void parseKeyValuePair(YiniSection &section);
  void parseQuickRegistration(YiniSection &section);
  YiniValue parseValue();
  std::unique_ptr<YiniArray> parseArray();
  std::unique_ptr<YiniArray> parseArrayFromFunction();
  std::unique_ptr<YiniList> parseList();
  std::unique_ptr<YiniSet> parseSet();
  YiniVariant parseMap();
  YiniValue parseExpression();
  YiniValue parseTerm();
  YiniValue parseFactor();
  std::unique_ptr<YiniCoord> parseCoord();
  std::unique_ptr<YiniColor> parseColor();
  std::unique_ptr<YiniPath> parsePath();

  Lexer lexer;
  Token currentToken;
  YiniDocument &document;
  std::string basePath;
  std::vector<YiniSyntaxError> m_errors;

  struct ParseError {};

  void nextToken();
};
} // namespace YINI

#endif // YINI_PARSER_HPP