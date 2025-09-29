#ifndef YINI_PARSER_HPP
#define YINI_PARSER_HPP

#include "Lexer.hpp"
#include "YiniData.hpp"
#include <memory>
#include <string>

namespace YINI
{
class Parser
{
public:
  Parser(const std::string &content, YiniDocument &document,
         const std::string &basePath = ".");
  void parse();

private:
  void parseSection();
  void parseKeyValuePair(YiniSection &section);
  void parseQuickRegistration(YiniSection &section);
  YiniValue parseValue();
  std::unique_ptr<YiniArray> parseArray();
  std::unique_ptr<YiniMap> parseMap();
  YiniValue parseExpression();
  YiniValue parseTerm();
  YiniValue parseFactor();
  std::unique_ptr<YiniCoord> parseCoord();
  std::unique_ptr<YiniColor> parseColor();
  std::unique_ptr<YiniPath> parsePath();

  Lexer m_lexer;
  Token m_currentToken;
  YiniDocument &m_document;
  std::string m_basePath;
  void nextToken();
};
} // namespace YINI

#endif // YINI_PARSER_HPP