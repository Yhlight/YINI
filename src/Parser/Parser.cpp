#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <variant>

namespace YINI
{

static std::string read_file_content_internal(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open()) return "";
  return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

namespace {
bool is_numeric(const YiniValue &val) {
  return std::holds_alternative<int>(val.data) || std::holds_alternative<double>(val.data);
}

YiniValue apply_op(const YiniValue &left, const YiniValue &right, TokenType op)
{
  if (!is_numeric(left) || !is_numeric(right)) return {};
  YiniValue result;
  bool use_double = std::holds_alternative<double>(left.data) || std::holds_alternative<double>(right.data);
  double left_d = std::holds_alternative<int>(left.data) ? static_cast<double>(std::get<int>(left.data)) : std::get<double>(left.data);
  double right_d = std::holds_alternative<int>(right.data) ? static_cast<double>(std::get<int>(right.data)) : std::get<double>(right.data);

  if (use_double) {
      switch (op) {
          case TokenType::Plus: result.data = left_d + right_d; break;
          case TokenType::Minus: result.data = left_d - right_d; break;
          case TokenType::Star: result.data = left_d * right_d; break;
          case TokenType::Slash: result.data = right_d != 0 ? left_d / right_d : 0.0; break;
          default: break;
      }
  } else {
      int left_i = std::get<int>(left.data);
      int right_i = std::get<int>(right.data);
      switch (op) {
          case TokenType::Plus: result.data = left_i + right_i; break;
          case TokenType::Minus: result.data = left_i - right_i; break;
          case TokenType::Star: result.data = left_i * right_i; break;
          case TokenType::Slash: result.data = right_i != 0 ? left_i / right_i : 0; break;
          case TokenType::Percent: result.data = right_i != 0 ? left_i % right_i : 0; break;
          default: break;
      }
  }
  return result;
}
}

Parser::Parser(const std::string &content, YiniDocument &doc, const std::string &base_path)
    : lexer(content), document(doc), basePath(base_path)
{
    nextToken();
}

void Parser::nextToken() { currentToken = lexer.getNextToken(); }

bool Parser::hadError() const { return !m_errors.empty(); }

const std::vector<YiniSyntaxError>& Parser::getErrors() const { return m_errors; }

void Parser::reportError(const std::string& message) {
    if (m_errors.empty() || m_errors.back().line != currentToken.line) {
        m_errors.push_back({message, currentToken.line, currentToken.column});
    }
}

void Parser::synchronize() {
    nextToken();
    while (currentToken.type != TokenType::Eof) {
        if (currentToken.type == TokenType::LeftBracket) return;
        nextToken();
    }
}

void Parser::parse()
{
    while (currentToken.type != TokenType::Eof) {
        try {
            if (currentToken.type == TokenType::LeftBracket) {
                parseSection();
            } else {
                reportError("Unexpected token at root level.");
                synchronize();
            }
        } catch (const ParseError&) {
            synchronize();
        }
    }

    if (hadError()) {
        throw YiniParsingException(m_errors);
    }
}

void Parser::parseSection()
{
  nextToken();

  bool is_define_section = false;
  bool is_include_section = false;

  if (currentToken.type == TokenType::Hash)
  {
    nextToken();
    if (currentToken.type == TokenType::Identifier && currentToken.value == "define")
    {
      is_define_section = true;
      nextToken();
    }
    else if (currentToken.type == TokenType::Identifier && currentToken.value == "include")
    {
      is_include_section = true;
      nextToken();
    }
  }

  if (is_define_section)
  {
    if (currentToken.type != TokenType::RightBracket)
    {
      reportError("Expected ']' to close #define directive.");
      throw ParseError();
    }
    nextToken();
    while (currentToken.type != TokenType::LeftBracket && currentToken.type != TokenType::Eof)
    {
      if (currentToken.type == TokenType::Identifier)
      {
        std::string key = currentToken.value;
        int line = currentToken.line;
        int column = currentToken.column;
        nextToken();
        if (currentToken.type == TokenType::Equals)
        {
          nextToken();
          document.addDefine(key, parseValue(), line, column);
        }
        else
        {
          nextToken();
        }
      }
      else
      {
        nextToken();
      }
    }
  }
  else if (is_include_section)
  {
    if (currentToken.type != TokenType::RightBracket)
    {
        reportError("Expected ']' to close #include directive.");
        throw ParseError();
    }
    nextToken();
    while (currentToken.type != TokenType::LeftBracket && currentToken.type != TokenType::Eof)
    {
      if (currentToken.type == TokenType::PlusEquals)
      {
        nextToken();
        std::string file_to_include;
        while (currentToken.type != TokenType::LeftBracket && currentToken.type != TokenType::Eof && currentToken.type != TokenType::PlusEquals)
        {
          file_to_include += currentToken.value;
          nextToken();
        }

        if (!file_to_include.empty())
        {
          file_to_include.erase(file_to_include.find_last_not_of(" \t\n\r") + 1);
          std::string full_path = basePath + "/" + file_to_include;
          std::string included_content = read_file_content_internal(full_path);
          if (!included_content.empty())
          {
            Parser sub_parser(included_content, document, basePath);
            sub_parser.parse();
             if (sub_parser.hadError()) {
                for (const auto& err : sub_parser.getErrors()) {
                    m_errors.push_back(err);
                }
            }
          }
        }
      }
      else
      {
        nextToken();
      }
    }
  }
  else
  {
    std::string section_name_val;
    if (currentToken.type == TokenType::Identifier)
    {
      section_name_val = currentToken.value;
      nextToken();
    }
    else
    {
      reportError("Invalid section name.");
      throw ParseError();
    }

    YiniSection *section = document.getOrCreateSection(section_name_val);

    if (currentToken.type == TokenType::Colon)
    {
      nextToken();
      while (currentToken.type == TokenType::Identifier)
      {
        section->inheritedSections.push_back(currentToken.value);
        nextToken();
        if (currentToken.type == TokenType::Comma) { nextToken(); }
        else { break; }
      }
    }

    if (currentToken.type != TokenType::RightBracket)
    {
        reportError("Expected ']' to close section header.");
        throw ParseError();
    }
    nextToken();

    while (currentToken.type != TokenType::LeftBracket && currentToken.type != TokenType::Eof)
    {
        try {
            if (currentToken.type == TokenType::Identifier)
            {
                parseKeyValuePair(*section);
            }
            else if (currentToken.type == TokenType::PlusEquals)
            {
                parseQuickRegistration(*section);
            }
            else
            {
                reportError("Expected a key-value pair or quick registration.");
                throw ParseError();
            }
        } catch(const ParseError&) {
            synchronize();
            if (currentToken.type == TokenType::LeftBracket || currentToken.type == TokenType::Eof) {
                break;
            }
        }
    }
  }
}

void Parser::parseKeyValuePair(YiniSection &section)
{
  YiniKeyValuePair pair;
  pair.key = currentToken.value;
  nextToken();

  if (currentToken.type != TokenType::Equals)
  {
    reportError("Expected '=' after key '" + pair.key + "'.");
    throw ParseError();
  }

  nextToken();
  YiniValue parsed_value = parseValue();

  if (std::holds_alternative<std::unique_ptr<YiniDynaValue>>(parsed_value.data))
  {
    pair.is_dynamic = true;
    auto& dyna_ptr = std::get<std::unique_ptr<YiniDynaValue>>(parsed_value.data);
    pair.value = std::move(dyna_ptr->value);
  }
  else
  {
    pair.value = std::move(parsed_value);
  }

  auto it = std::find_if(section.pairs.begin(), section.pairs.end(),
      [&](const YiniKeyValuePair &p) { return p.key == pair.key; });

  if (it != section.pairs.end())
  {
    *it = std::move(pair);
  }
  else
  {
    section.pairs.push_back(std::move(pair));
  }
}

void Parser::parseQuickRegistration(YiniSection &section)
{
  nextToken();
  section.registrationList.push_back(parseValue());
}

YiniValue Parser::parseValue()
{
  YiniValue val;
  switch (currentToken.type)
  {
  case TokenType::String:
    val.data = currentToken.value;
    nextToken();
    return val;
  case TokenType::Boolean:
    val.data = (currentToken.value == "true");
    nextToken();
    return val;
  case TokenType::LeftBracket:
    val.data = parseArray();
    return val;
  case TokenType::LeftBrace:
    val.data = parseMap();
    return val;
  case TokenType::HexColor:
    val.data = parseColor();
    return val;
  case TokenType::Identifier:
  {
    std::string id_val = currentToken.value;
    std::transform(id_val.begin(), id_val.end(), id_val.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (id_val == "dyna")
    {
      nextToken();
      if (currentToken.type != TokenType::LeftParen) {
        reportError("Expected '(' after Dyna.");
        throw ParseError();
      }
      nextToken();
      auto dyna_val = std::make_unique<YiniDynaValue>();
      dyna_val->value = parseValue();
      if (currentToken.type != TokenType::RightParen) {
        reportError("Expected ')' to close Dyna expression.");
        throw ParseError();
      }
      nextToken();
      val.data = std::move(dyna_val);
      return val;
    }
    if (id_val == "array") { val.data = parseArrayFromFunction(); return val; }
    if (id_val == "coord") { val.data = parseCoord(); return val; }
    if (id_val == "list") { val.data = parseList(); return val; }
    if (id_val == "set") { val.data = parseSet(); return val; }
    if (id_val == "color") { val.data = parseColor(); return val; }
    if (id_val == "path") { val.data = parsePath(); return val; }
  }
  case TokenType::Number:
  case TokenType::At:
  case TokenType::LeftParen:
  case TokenType::Minus:
    return parseExpression();
  default:
    reportError("Unexpected token when parsing value.");
    throw ParseError();
  }
}

YiniValue Parser::parseFactor()
{
  YiniValue result;
  switch (currentToken.type)
  {
  case TokenType::Number:
    if (currentToken.value.find('.') != std::string::npos)
      result.data = std::stod(currentToken.value);
    else
      result.data = std::stoi(currentToken.value);
    nextToken();
    break;
  case TokenType::LeftParen:
    nextToken();
    result = parseExpression();
    if (currentToken.type != TokenType::RightParen) {
        reportError("Expected ')' to close expression.");
        throw ParseError();
    }
    nextToken();
    break;
  case TokenType::At:
    nextToken();
    if (currentToken.type == TokenType::Identifier)
    {
      YiniDefine define;
      if (!document.getDefine(currentToken.value, define))
      {
        reportError("Undefined macro: " + currentToken.value);
        throw ParseError();
      }
      result = define.value;
      nextToken();
    }
    break;
  case TokenType::Minus:
    nextToken();
    result = parseFactor();
    if (is_numeric(result))
    {
      if (std::holds_alternative<double>(result.data))
        result.data = -std::get<double>(result.data);
      else
        result.data = -std::get<int>(result.data);
    }
    break;
  default:
    nextToken();
    break;
  }
  return result;
}

YiniValue Parser::parseTerm()
{
  YiniValue result = parseFactor();
  while (currentToken.type == TokenType::Star ||
         currentToken.type == TokenType::Slash ||
         currentToken.type == TokenType::Percent)
  {
    Token op = currentToken;
    nextToken();
    YiniValue right = parseFactor();
    result = apply_op(result, right, op.type);
  }
  return result;
}

YiniValue Parser::parseExpression()
{
  YiniValue result = parseTerm();
  while (currentToken.type == TokenType::Plus ||
         currentToken.type == TokenType::Minus)
  {
    Token op = currentToken;
    nextToken();
    YiniValue right = parseTerm();
    result = apply_op(result, right, op.type);
  }
  return result;
}

std::unique_ptr<YiniArray> Parser::parseArray()
{
  auto arr = std::make_unique<YiniArray>();
  nextToken();

  while (currentToken.type != TokenType::RightBracket &&
         currentToken.type != TokenType::Eof)
  {
    arr->elements.push_back(parseValue());
    if (currentToken.type == TokenType::Comma) { nextToken(); }
  }

  if (currentToken.type != TokenType::RightBracket)
  {
    reportError("Expected ']' to close array.");
    throw ParseError();
  }
  nextToken();
  return arr;
}

std::unique_ptr<YiniList> Parser::parseList()
{
  nextToken();
  if (currentToken.type != TokenType::LeftParen){
    reportError("Expected '(' after List.");
    throw ParseError();
  }
  nextToken();

  auto list = std::make_unique<YiniList>();

  while (currentToken.type != TokenType::RightParen &&
         currentToken.type != TokenType::Eof)
  {
    list->elements.push_back(parseValue());
    if (currentToken.type == TokenType::Comma) { nextToken(); }
  }

  if (currentToken.type != TokenType::RightParen)
  {
    reportError("Expected ')' to close list.");
    throw ParseError();
  }
  nextToken();
  return list;
}

std::unique_ptr<YiniArray> Parser::parseArrayFromFunction()
{
  nextToken();
  if (currentToken.type != TokenType::LeftParen){
    reportError("Expected '(' after Array.");
    throw ParseError();
  }
  nextToken();

  auto arr = std::make_unique<YiniArray>();

  while (currentToken.type != TokenType::RightParen &&
         currentToken.type != TokenType::Eof)
  {
    arr->elements.push_back(parseValue());
    if (currentToken.type == TokenType::Comma) { nextToken(); }
  }

  if (currentToken.type != TokenType::RightParen)
  {
    reportError("Expected ')' to close Array expression.");
    throw ParseError();
  }
  nextToken();
  return arr;
}

std::unique_ptr<YiniSet> Parser::parseSet()
{
  nextToken();
  if (currentToken.type != TokenType::LeftParen){
    reportError("Expected '(' after Set.");
    throw ParseError();
  }
  nextToken();

  auto set = std::make_unique<YiniSet>();

  while (currentToken.type != TokenType::RightParen &&
         currentToken.type != TokenType::Eof)
  {
    set->elements.insert(parseValue());

    if (currentToken.type == TokenType::Comma) { nextToken(); }
    else if (currentToken.type != TokenType::RightParen)
    {
      reportError("Expected ',' or ')' in Set expression.");
      throw ParseError();
    }
  }

  if (currentToken.type != TokenType::RightParen)
  {
    reportError("Expected ')' to close Set expression.");
    throw ParseError();
  }
  nextToken();
  return set;
}

YiniVariant Parser::parseMap()
{
  nextToken(); // consume '{'

  std::map<std::string, YiniValue> temp_map;

  while (currentToken.type != TokenType::RightBrace &&
         currentToken.type != TokenType::Eof)
  {
    if (currentToken.type != TokenType::String &&
        currentToken.type != TokenType::Identifier)
    {
      reportError("Expected a string or identifier as a map key.");
      throw ParseError();
    }
    std::string key = currentToken.value;
    nextToken();

    if (currentToken.type != TokenType::Colon)
    {
      reportError("Expected ':' after map key.");
      throw ParseError();
    }
    nextToken();

    temp_map[key] = parseValue();

    if (currentToken.type == TokenType::Comma) { nextToken(); }
    else if (currentToken.type != TokenType::RightBrace)
    {
      reportError("Expected ',' or '}' in map.");
      throw ParseError();
    }
  }

  if (currentToken.type != TokenType::RightBrace)
  {
    reportError("Expected '}' to close map.");
    throw ParseError();
  }
  nextToken(); // consume '}'

  if (temp_map.size() == 1)
  {
      auto tuple = std::make_unique<YiniTuple>();
      tuple->key = temp_map.begin()->first;
      tuple->value = std::move(temp_map.begin()->second);
      return tuple;
  }
  else
  {
      auto map = std::make_unique<YiniMap>();
      map->elements = std::move(temp_map);
      return map;
  }
}

std::unique_ptr<YiniCoord> Parser::parseCoord()
{
  nextToken();
  if (currentToken.type != TokenType::LeftParen){
    reportError("Expected '(' after Coord.");
    throw ParseError();
  }
  nextToken();

  auto coord = std::make_unique<YiniCoord>();

  YiniValue x_val = parseExpression();
  if (std::holds_alternative<int>(x_val.data))
    coord->x = std::get<int>(x_val.data);
  else if (std::holds_alternative<double>(x_val.data))
    coord->x = std::get<double>(x_val.data);
  else {
    reportError("Coord parameters must be numeric.");
    throw ParseError();
  }

  if (currentToken.type != TokenType::Comma){
    reportError("Expected ',' in Coord.");
    throw ParseError();
  }
  nextToken();
  YiniValue y_val = parseExpression();
  if (std::holds_alternative<int>(y_val.data))
    coord->y = std::get<int>(y_val.data);
  else if (std::holds_alternative<double>(y_val.data))
    coord->y = std::get<double>(y_val.data);
  else {
    reportError("Coord parameters must be numeric.");
    throw ParseError();
  }

  if (currentToken.type == TokenType::Comma)
  {
    nextToken();
    YiniValue z_val = parseExpression();
    if (std::holds_alternative<int>(z_val.data))
      coord->z = std::get<int>(z_val.data);
    else if (std::holds_alternative<double>(z_val.data))
      coord->z = std::get<double>(z_val.data);
    else {
      reportError("Coord parameters must be numeric.");
      throw ParseError();
    }
    coord->is_3d = true;
  }
  else
  {
    coord->z = 0;
    coord->is_3d = false;
  }

  if (currentToken.type != TokenType::RightParen) {
    reportError("Expected ')' to close Coord expression.");
    throw ParseError();
  }
  nextToken();
  return coord;
}

std::unique_ptr<YiniColor> Parser::parseColor()
{
  if (currentToken.type == TokenType::HexColor)
  {
    auto color = std::make_unique<YiniColor>();
    unsigned int r, g, b;
    sscanf(currentToken.value.c_str(), "%2x%2x%2x", &r, &g, &b);
    color->r = r;
    color->g = g;
    color->b = b;
    nextToken();
    return color;
  }

  nextToken();
  if (currentToken.type != TokenType::LeftParen){
    reportError("Expected '(' after Color.");
    throw ParseError();
  }
  nextToken();

  auto color = std::make_unique<YiniColor>();

  YiniValue r_val = parseExpression();
  if (!std::holds_alternative<int>(r_val.data)) {
    reportError("Color parameters must be integers.");
    throw ParseError();
  }
  color->r = std::get<int>(r_val.data);

  if (currentToken.type != TokenType::Comma) {
    reportError("Expected ',' in Color.");
    throw ParseError();
  }
  nextToken();
  YiniValue g_val = parseExpression();
  if (!std::holds_alternative<int>(g_val.data)) {
    reportError("Color parameters must be integers.");
    throw ParseError();
  }
  color->g = std::get<int>(g_val.data);

  if (currentToken.type != TokenType::Comma) {
    reportError("Expected ',' in Color.");
    throw ParseError();
  }
  nextToken();
  YiniValue b_val = parseExpression();
  if (!std::holds_alternative<int>(b_val.data)) {
    reportError("Color parameters must be integers.");
    throw ParseError();
  }
  color->b = std::get<int>(b_val.data);

  if (currentToken.type != TokenType::RightParen) {
    reportError("Expected ')' to close Color expression.");
    throw ParseError();
  }
  nextToken();
  return color;
}

std::unique_ptr<YiniPath> Parser::parsePath()
{
  nextToken();
  if (currentToken.type != TokenType::LeftParen) {
    reportError("Expected '(' after Path.");
    throw ParseError();
  }
  nextToken();

  auto path = std::make_unique<YiniPath>();

  std::string path_str;
  while (currentToken.type != TokenType::RightParen &&
         currentToken.type != TokenType::Eof)
  {
    path_str += currentToken.value;
    nextToken();
  }
  path->path_value = path_str;

  if (currentToken.type != TokenType::RightParen) {
    reportError("Expected ')' to close Path expression.");
    throw ParseError();
  }
  nextToken();
  return path;
}

}