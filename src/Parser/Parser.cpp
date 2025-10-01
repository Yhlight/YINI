#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include "YINI/fs_compat.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <variant>

static std::string read_file_content_internal(const std::string &path)
{
  std::ifstream t(path);
  if (!t.is_open())
  {
    return "";
  }
  return std::string((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
}

namespace
{ // Anonymous namespace for helpers

bool is_numeric(const YINI::YiniValue &val)
{
  return std::holds_alternative<int>(val.data) ||
         std::holds_alternative<double>(val.data);
}

YINI::YiniValue apply_op(const YINI::YiniValue &left,
                         const YINI::YiniValue &right, YINI::TokenType op)
{
  if (!is_numeric(left) || !is_numeric(right))
    return {};

  YINI::YiniValue result;
  bool use_double = std::holds_alternative<double>(left.data) ||
                    std::holds_alternative<double>(right.data);

  double left_d = std::holds_alternative<int>(left.data)
                      ? static_cast<double>(std::get<int>(left.data))
                      : std::get<double>(left.data);
  double right_d = std::holds_alternative<int>(right.data)
                       ? static_cast<double>(std::get<int>(right.data))
                       : std::get<double>(right.data);

  if (use_double)
  {
    switch (op)
    {
    case YINI::TokenType::Plus:
      result.data = left_d + right_d;
      break;
    case YINI::TokenType::Minus:
      result.data = left_d - right_d;
      break;
    case YINI::TokenType::Star:
      result.data = left_d * right_d;
      break;
    case YINI::TokenType::Slash:
      result.data = right_d != 0 ? left_d / right_d : 0.0;
      break;
    default:
      break;
    }
  }
  else
  {
    int left_i = std::get<int>(left.data);
    int right_i = std::get<int>(right.data);
    switch (op)
    {
    case YINI::TokenType::Plus:
      result.data = left_i + right_i;
      break;
    case YINI::TokenType::Minus:
      result.data = left_i - right_i;
      break;
    case YINI::TokenType::Star:
      result.data = left_i * right_i;
      break;
    case YINI::TokenType::Slash:
      result.data = right_i != 0 ? left_i / right_i : 0;
      break;
    case YINI::TokenType::Percent:
      result.data = right_i != 0 ? left_i % right_i : 0;
      break;
    default:
      break;
    }
  }
  return result;
}

} // end anonymous namespace

namespace YINI
{
Parser::Parser(const std::string &content, YiniDocument &document,
               const std::string &basePath)
    : m_lexer(content), m_document(document), m_has_lookahead(false),
      m_base_path(basePath)
{
  nextToken();
}

void Parser::nextToken()
{
  if (m_has_lookahead)
  {
    m_current_token = m_lookahead_token;
    m_has_lookahead = false;
  }
  else
  {
    m_current_token = m_lexer.getNextToken();
  }
}

Token Parser::peekToken()
{
  if (!m_has_lookahead)
  {
    m_lookahead_token = m_lexer.getNextToken();
    m_has_lookahead = true;
  }
  return m_lookahead_token;
}

void Parser::parse()
{
  while (m_current_token.type != TokenType::Eof)
  {
    if (m_current_token.type == TokenType::LeftBracket)
    {
      parseSection();
    }
    else
    {
      if (m_current_token.type != TokenType::Eof)
      {
        throw YiniException("Unexpected token at root level.",
                            m_current_token.line, m_current_token.column);
      }
      nextToken();
    }
  }
}

void Parser::parseSection()
{
  nextToken();

  bool is_define_section = false;
  bool is_include_section = false;

  if (m_current_token.type == TokenType::Hash)
  {
    nextToken();
    if (m_current_token.type == TokenType::Identifier &&
        m_current_token.value == "define")
    {
      is_define_section = true;
      nextToken();
    }
    else if (m_current_token.type == TokenType::Identifier &&
             m_current_token.value == "include")
    {
      is_include_section = true;
      nextToken();
    }
  }

  if (is_define_section)
  {
    if (m_current_token.type != TokenType::RightBracket)
    {
      throw YiniException("Expected ']' to close #define directive.",
                          m_current_token.line, m_current_token.column);
    }
    nextToken();
    while (m_current_token.type != TokenType::LeftBracket &&
           m_current_token.type != TokenType::Eof)
    {
      if (m_current_token.type == TokenType::Identifier)
      {
        std::string key = m_current_token.value;
        nextToken();
        if (m_current_token.type == TokenType::Equals)
        {
          nextToken();
          m_document.addDefine(key, parseValue());
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
    if (m_current_token.type != TokenType::RightBracket)
    {
      throw YiniException("Expected ']' to close #include directive.",
                          m_current_token.line, m_current_token.column);
    }
    nextToken();
    while (m_current_token.type != TokenType::LeftBracket &&
           m_current_token.type != TokenType::Eof)
    {
      if (m_current_token.type == TokenType::PlusEquals)
      {
        nextToken();
        if (m_current_token.type == TokenType::Identifier)
        {
          std::string file_to_include = m_current_token.value;

          fs::path path_base(m_base_path);
          fs::path combined_path = path_base / file_to_include;
          fs::path path_full = combined_path.lexically_normal();

          std::string included_content =
              read_file_content_internal(path_full.string());
          if (!included_content.empty())
          {
            fs::path new_base = path_full.parent_path();
            Parser sub_parser(included_content, m_document, new_base.string());
            sub_parser.parse();
          }
          nextToken();
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
  else
  {
    std::string section_name_val;
    if (m_current_token.type == TokenType::Identifier)
    {
      section_name_val = m_current_token.value;
      nextToken();
    }
    else
    {
      throw YiniException("Invalid section name.", m_current_token.line,
                          m_current_token.column);
    }

    YiniSection *section = m_document.getOrCreateSection(section_name_val);

    if (m_current_token.type == TokenType::Colon)
    {
      nextToken();
      while (m_current_token.type == TokenType::Identifier)
      {
        section->inheritedSections.push_back(m_current_token.value);
        nextToken();
        if (m_current_token.type == TokenType::Comma)
        {
          nextToken();
        }
        else
        {
          break;
        }
      }
    }

    if (m_current_token.type != TokenType::RightBracket)
    {
      throw YiniException("Expected ']' to close section header.",
                          m_current_token.line, m_current_token.column);
    }
    nextToken();

    while (m_current_token.type != TokenType::LeftBracket &&
           m_current_token.type != TokenType::Eof)
    {
      if (m_current_token.type == TokenType::Identifier)
      {
        parseKeyValuePair(*section);
      }
      else if (m_current_token.type == TokenType::PlusEquals)
      {
        parseQuickRegistration(*section);
      }
      else
      {
        nextToken();
      }
    }
  }
}

void Parser::parseKeyValuePair(YiniSection &section)
{
  YiniKeyValuePair pair;
  pair.key = m_current_token.value;
  nextToken();

  if (m_current_token.type == TokenType::Equals)
  {
    nextToken();
    pair.value = parseValue();

    auto it = std::find_if(section.pairs.begin(), section.pairs.end(),
                           [&](const YiniKeyValuePair &p)
                           { return p.key == pair.key; });

    if (it != section.pairs.end())
    {
      it->value = pair.value;
    }
    else
    {
      section.pairs.push_back(std::move(pair));
    }
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
  switch (m_current_token.type)
  {
  case TokenType::String:
    val.data = m_current_token.value;
    nextToken();
    return val;
  case TokenType::Boolean:
    val.data = (m_current_token.value == "true");
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
    std::string id_val = m_current_token.value;
    std::transform(id_val.begin(), id_val.end(), id_val.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (id_val == "dyna")
    {
      val.data = parseDyna();
      return val;
    }
    if (id_val == "array")
    {
      val.data = parseArrayFromFunction();
      return val;
    }
    if (id_val == "coord")
    {
      val.data = parseCoord();
      return val;
    }
    if (id_val == "list")
    {
      val.data = parseList();
      return val;
    }
    if (id_val == "color")
    {
      val.data = parseColor();
      return val;
    }
    if (id_val == "path")
    {
      val.data = parsePath();
      return val;
    }
    throw YiniException("Unexpected identifier '" + m_current_token.value + "' when parsing a value.",
                        m_current_token.line, m_current_token.column);
  }
  case TokenType::Number:
  case TokenType::At:
  case TokenType::Minus:
    return parseExpression();
  case TokenType::LeftParen:
    return parseParenthesized();
  default:
    throw YiniException("Unexpected token when parsing value.",
                        m_current_token.line, m_current_token.column);
  }
}

std::unique_ptr<YiniDynaValue> Parser::parseDyna() {
    nextToken(); // consume 'Dyna'
    if (m_current_token.type != TokenType::LeftParen)
        throw YiniException("Expected '(' after Dyna.", m_current_token.line,
                            m_current_token.column);
    nextToken(); // consume '('
    auto dyna_val = std::make_unique<YiniDynaValue>();
    dyna_val->value = parseValue();
    if (m_current_token.type != TokenType::RightParen)
        throw YiniException("Expected ')' to close Dyna expression.",
                            m_current_token.line, m_current_token.column);
    nextToken(); // consume ')'
    return dyna_val;
}

YiniValue Parser::parseParenthesized() {
    nextToken(); // Consume '('

    if (m_current_token.type == TokenType::RightParen) { // Empty set ()
        nextToken(); // Consume ')'
        YiniValue val;
        val.data = std::make_unique<YiniSet>();
        return val;
    }

    YiniValue first_val = parseValue();

    if (m_current_token.type == TokenType::Comma) { // It's a set
        auto set = std::make_unique<YiniSet>();
        auto it_first =
            std::find_if(set->elements.begin(), set->elements.end(),
                         [&](const YiniValue &elem) { return elem == first_val; });
        if (it_first == set->elements.end()) {
            set->elements.push_back(std::move(first_val));
        }

        while (m_current_token.type == TokenType::Comma) {
            nextToken(); // Consume ','
            if (m_current_token.type == TokenType::RightParen) break; // Trailing comma

            YiniValue next_val = parseValue();
            auto it = std::find_if(set->elements.begin(), set->elements.end(),
                                   [&](const YiniValue &elem) { return elem == next_val; });
            if (it == set->elements.end()) {
                set->elements.push_back(std::move(next_val));
            }
        }

        if (m_current_token.type != TokenType::RightParen) {
            throw YiniException("Expected ')' to close set.", m_current_token.line, m_current_token.column);
        }
        nextToken(); // Consume ')'
        YiniValue val;
        val.data = std::move(set);
        return val;

    } else if (m_current_token.type == TokenType::RightParen) { // It was a single parenthesized value
        nextToken(); // Consume ')'
        return first_val;
    } else {
        throw YiniException("Expected ',' or ')' in parenthesized expression.", m_current_token.line, m_current_token.column);
    }
}

YiniValue Parser::parseFactor()
{
  YiniValue result;
  switch (m_current_token.type)
  {
  case TokenType::Number:
    if (m_current_token.value.find('.') != std::string::npos)
      result.data = std::stod(m_current_token.value);
    else
      result.data = std::stoi(m_current_token.value);
    nextToken();
    break;
  case TokenType::LeftParen:
    nextToken();
    result = parseExpression();
    if (m_current_token.type != TokenType::RightParen)
      throw YiniException("Expected ')' to close expression.",
                          m_current_token.line, m_current_token.column);
    nextToken();
    break;
  case TokenType::At:
    nextToken();
    if (m_current_token.type == TokenType::Identifier)
    {
      if (!m_document.getDefine(m_current_token.value, result))
      {
        throw YiniException("Undefined macro: " + m_current_token.value,
                            m_current_token.line, m_current_token.column);
      }
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
  while (m_current_token.type == TokenType::Star ||
         m_current_token.type == TokenType::Slash ||
         m_current_token.type == TokenType::Percent)
  {
    Token op = m_current_token;
    nextToken();
    YiniValue right = parseFactor();
    result = apply_op(result, right, op.type);
  }
  return result;
}

YiniValue Parser::parseExpression()
{
  YiniValue result = parseTerm();
  while (m_current_token.type == TokenType::Plus ||
         m_current_token.type == TokenType::Minus)
  {
    Token op = m_current_token;
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

  while (m_current_token.type != TokenType::RightBracket &&
         m_current_token.type != TokenType::Eof)
  {
    arr->elements.push_back(parseValue());

    if (m_current_token.type == TokenType::Comma)
    {
      nextToken();
    }
  }

  if (m_current_token.type != TokenType::RightBracket)
  {
    throw YiniException("Expected ']' to close array.", m_current_token.line,
                        m_current_token.column);
  }
  nextToken();
  return arr;
}

std::unique_ptr<YiniList> Parser::parseList()
{
  nextToken(); // consume 'List'
  if (m_current_token.type != TokenType::LeftParen)
    throw YiniException("Expected '(' after List.", m_current_token.line,
                        m_current_token.column);
  nextToken(); // consume '('

  auto list = std::make_unique<YiniList>();

  while (m_current_token.type != TokenType::RightParen &&
         m_current_token.type != TokenType::Eof)
  {
    list->elements.push_back(parseValue());

    if (m_current_token.type == TokenType::Comma)
    {
      nextToken();
    }
  }

  if (m_current_token.type != TokenType::RightParen)
  {
    throw YiniException("Expected ')' to close list.", m_current_token.line,
                        m_current_token.column);
  }
  nextToken(); // consume ')'
  return list;
}

std::unique_ptr<YiniArray> Parser::parseArrayFromFunction()
{
  nextToken(); // consume 'Array'
  if (m_current_token.type != TokenType::LeftParen)
    throw YiniException("Expected '(' after Array.", m_current_token.line,
                        m_current_token.column);
  nextToken(); // consume '('

  auto arr = std::make_unique<YiniArray>();

  while (m_current_token.type != TokenType::RightParen &&
         m_current_token.type != TokenType::Eof)
  {
    arr->elements.push_back(parseValue());

    if (m_current_token.type == TokenType::Comma)
    {
      nextToken();
    }
  }

  if (m_current_token.type != TokenType::RightParen)
  {
    throw YiniException("Expected ')' to close Array expression.", m_current_token.line,
                        m_current_token.column);
  }
  nextToken(); // consume ')'
  return arr;
}

std::unique_ptr<YiniMap> Parser::parseMap()
{
  auto map = std::make_unique<YiniMap>();
  nextToken(); // consume '{'

  while (m_current_token.type != TokenType::RightBrace &&
         m_current_token.type != TokenType::Eof)
  {
    // Key (must be a string or identifier)
    if (m_current_token.type != TokenType::String &&
        m_current_token.type != TokenType::Identifier)
    {
      throw YiniException("Expected a string or identifier as a map key.",
                          m_current_token.line, m_current_token.column);
    }
    std::string key = m_current_token.value;
    nextToken();

    // Colon
    if (m_current_token.type != TokenType::Colon)
    {
      throw YiniException("Expected ':' after map key.", m_current_token.line,
                          m_current_token.column);
    }
    nextToken();

    // Value
    map->elements[key] = parseValue();

    // Comma or closing brace
    if (m_current_token.type == TokenType::Comma)
    {
      nextToken();
    }
    else if (m_current_token.type != TokenType::RightBrace)
    {
      throw YiniException("Expected ',' or '}' in map.", m_current_token.line,
                          m_current_token.column);
    }
  }

  if (m_current_token.type != TokenType::RightBrace)
  {
    throw YiniException("Expected '}' to close map.", m_current_token.line,
                        m_current_token.column);
  }
  nextToken(); // consume '}'
  return map;
}

std::unique_ptr<YiniCoord> Parser::parseCoord()
{
  nextToken(); // consume 'Coord'
  if (m_current_token.type != TokenType::LeftParen)
    throw YiniException("Expected '(' after Coord.", m_current_token.line,
                        m_current_token.column);
  nextToken(); // consume '('

  auto coord = std::make_unique<YiniCoord>();

  YiniValue x_val = parseExpression();
  if (std::holds_alternative<int>(x_val.data))
    coord->x = std::get<int>(x_val.data);
  else if (std::holds_alternative<double>(x_val.data))
    coord->x = std::get<double>(x_val.data);
  else
    throw YiniException("Coord parameters must be numeric.",
                        m_current_token.line, m_current_token.column);

  if (m_current_token.type != TokenType::Comma)
    throw YiniException("Expected ',' in Coord.", m_current_token.line,
                        m_current_token.column);
  nextToken();
  YiniValue y_val = parseExpression();
  if (std::holds_alternative<int>(y_val.data))
    coord->y = std::get<int>(y_val.data);
  else if (std::holds_alternative<double>(y_val.data))
    coord->y = std::get<double>(y_val.data);
  else
    throw YiniException("Coord parameters must be numeric.",
                        m_current_token.line, m_current_token.column);

  if (m_current_token.type == TokenType::Comma)
  {
    nextToken();
    YiniValue z_val = parseExpression();
    if (std::holds_alternative<int>(z_val.data))
      coord->z = std::get<int>(z_val.data);
    else if (std::holds_alternative<double>(z_val.data))
      coord->z = std::get<double>(z_val.data);
    else
      throw YiniException("Coord parameters must be numeric.",
                          m_current_token.line, m_current_token.column);
    coord->is_3d = true;
  }
  else
  {
    coord->z = 0;
    coord->is_3d = false;
  }

  if (m_current_token.type != TokenType::RightParen)
    throw YiniException("Expected ')' to close Coord expression.",
                        m_current_token.line, m_current_token.column);
  nextToken();
  return coord;
}

std::unique_ptr<YiniColor> Parser::parseColor()
{
  if (m_current_token.type == TokenType::HexColor)
  {
    auto color = std::make_unique<YiniColor>();
    unsigned int r, g, b;
    sscanf(m_current_token.value.c_str(), "%2x%2x%2x", &r, &g, &b);
    color->r = r;
    color->g = g;
    color->b = b;
    nextToken();
    return color;
  }

  nextToken(); // consume 'Color'
  if (m_current_token.type != TokenType::LeftParen)
    throw YiniException("Expected '(' after Color.", m_current_token.line,
                        m_current_token.column);
  nextToken(); // consume '('

  auto color = std::make_unique<YiniColor>();

  YiniValue r_val = parseExpression();
  if (!std::holds_alternative<int>(r_val.data))
    throw YiniException("Color parameters must be integers.",
                        m_current_token.line, m_current_token.column);
  color->r = std::get<int>(r_val.data);

  if (m_current_token.type != TokenType::Comma)
    throw YiniException("Expected ',' in Color.", m_current_token.line,
                        m_current_token.column);
  nextToken();
  YiniValue g_val = parseExpression();
  if (!std::holds_alternative<int>(g_val.data))
    throw YiniException("Color parameters must be integers.",
                        m_current_token.line, m_current_token.column);
  color->g = std::get<int>(g_val.data);

  if (m_current_token.type != TokenType::Comma)
    throw YiniException("Expected ',' in Color.", m_current_token.line,
                        m_current_token.column);
  nextToken();
  YiniValue b_val = parseExpression();
  if (!std::holds_alternative<int>(b_val.data))
    throw YiniException("Color parameters must be integers.",
                        m_current_token.line, m_current_token.column);
  color->b = std::get<int>(b_val.data);

  if (m_current_token.type != TokenType::RightParen)
    throw YiniException("Expected ')' to close Color expression.",
                        m_current_token.line, m_current_token.column);
  nextToken();
  return color;
}

std::unique_ptr<YiniPath> Parser::parsePath()
{
  nextToken(); // consume 'Path'
  if (m_current_token.type != TokenType::LeftParen)
    throw YiniException("Expected '(' after Path.", m_current_token.line,
                        m_current_token.column);
  nextToken(); // consume '('

  auto path = std::make_unique<YiniPath>();

  if (m_current_token.type != TokenType::String)
  {
      throw YiniException("Expected a string literal for Path value.", m_current_token.line, m_current_token.column);
  }

  path->pathValue = m_current_token.value;
  nextToken(); // consume string

  if (m_current_token.type != TokenType::RightParen)
    throw YiniException("Expected ')' to close Path expression.",
                        m_current_token.line, m_current_token.column);
  nextToken();
  return path;
}
} // namespace YINI