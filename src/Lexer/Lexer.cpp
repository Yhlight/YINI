#include "YINI/Lexer.hpp"
#include <cctype>

namespace YINI
{
Lexer::Lexer(const std::string &input)
    : m_input_str(input), m_position(0), m_line_num(1), m_column_num(1)
{
}

Token Lexer::getNextToken()
{
  skipWhitespace();

  if (m_position >= m_input_str.length())
  {
    return {TokenType::Eof, "", m_line_num, m_column_num};
  }

  char current_char = m_input_str[m_position];

  if (current_char == '/')
  {
    if (m_position + 1 < m_input_str.length())
    {
      if (m_input_str[m_position + 1] == '/')
      {
        skipComment();
        return getNextToken();
      }
      else if (m_input_str[m_position + 1] == '*')
      {
        skipBlockComment();
        return getNextToken();
      }
    }
  }

  if (current_char == '[')
  {
    m_position++;
    m_column_num++;
    return {TokenType::LeftBracket, "[", m_line_num, m_column_num - 1};
  }

  if (current_char == ']')
  {
    m_position++;
    m_column_num++;
    return {TokenType::RightBracket, "]", m_line_num, m_column_num - 1};
  }

  if (current_char == ',')
  {
    m_position++;
    m_column_num++;
    return {TokenType::Comma, ",", m_line_num, m_column_num - 1};
  }

  if (current_char == ':')
  {
    m_position++;
    m_column_num++;
    return {TokenType::Colon, ":", m_line_num, m_column_num - 1};
  }

  if (current_char == '=')
  {
    m_position++;
    m_column_num++;
    return {TokenType::Equals, "=", m_line_num, m_column_num - 1};
  }

  if (current_char == '+' && m_position + 1 < m_input_str.length() &&
      m_input_str[m_position + 1] == '=')
  {
    m_position += 2;
    m_column_num += 2;
    return {TokenType::PlusEquals, "+=", m_line_num, m_column_num - 2};
  }

  if (current_char == '@')
  {
    m_position++;
    m_column_num++;
    return {TokenType::At, "@", m_line_num, m_column_num - 1};
  }

  switch (current_char)
  {
  case '+':
    m_position++;
    m_column_num++;
    return {TokenType::Plus, "+", m_line_num, m_column_num - 1};
  case '-':
    m_position++;
    m_column_num++;
    return {TokenType::Minus, "-", m_line_num, m_column_num - 1};
  case '*':
    m_position++;
    m_column_num++;
    return {TokenType::Star, "*", m_line_num, m_column_num - 1};
  case '/':
    m_position++;
    m_column_num++;
    return {TokenType::Slash, "/", m_line_num, m_column_num - 1};
  case '%':
    m_position++;
    m_column_num++;
    return {TokenType::Percent, "%", m_line_num, m_column_num - 1};
  case '(':
    m_position++;
    m_column_num++;
    return {TokenType::LeftParen, "(", m_line_num, m_column_num - 1};
  case ')':
    m_position++;
    m_column_num++;
    return {TokenType::RightParen, ")", m_line_num, m_column_num - 1};
  case '{':
    m_position++;
    m_column_num++;
    return {TokenType::LeftBrace, "{", m_line_num, m_column_num - 1};
  case '}':
    m_position++;
    m_column_num++;
    return {TokenType::RightBrace, "}", m_line_num, m_column_num - 1};
  }

  if (current_char == '#')
  {
    // Check for hex color like #RRGGBB
    if (m_position + 7 <= m_input_str.length())
    {
      bool is_hex = true;
      for (int i = 1; i <= 6; ++i)
      {
        if (!isxdigit(m_input_str[m_position + i]))
        {
          is_hex = false;
          break;
        }
      }
      if (is_hex)
      {
        std::string hex_value;
        hex_value.reserve(6);
        for(int i = 1; i <= 6; ++i) {
            hex_value += m_input_str[m_position + i];
        }
        m_position += 7;
        m_column_num += 7;
        return {TokenType::HexColor, hex_value, m_line_num, m_column_num - 7};
      }
    }
    // Otherwise, it's a directive hash
    m_position++;
    m_column_num++;
    return {TokenType::Hash, "#", m_line_num, m_column_num - 1};
  }

  if (current_char == '"')
  {
    return parseString();
  }

  if (isdigit(current_char) ||
      (current_char == '-' && m_position + 1 < m_input_str.length() &&
       isdigit(m_input_str[m_position + 1])))
  {
    return parseNumber();
  }

  if (isalpha(current_char) || current_char == '_')
  {
    return parseIdentifier();
  }

  m_position++;
  m_column_num++;
  return {TokenType::Unknown, std::string(1, current_char), m_line_num,
          m_column_num - 1};
}

void Lexer::skipWhitespace()
{
  while (m_position < m_input_str.length() && isspace(m_input_str[m_position]))
  {
    if (m_input_str[m_position] == '\n')
    {
      m_line_num++;
      m_column_num = 1;
    }
    else
    {
      m_column_num++;
    }
    m_position++;
  }
}

void Lexer::skipComment()
{
  while (m_position < m_input_str.length() && m_input_str[m_position] != '\n')
  {
    m_position++;
    m_column_num++;
  }
}

void Lexer::skipBlockComment()
{
  m_position += 2; // Skip "/*"
  m_column_num += 2;

  while (m_position + 1 < m_input_str.length())
  {
    if (m_input_str[m_position] == '*' && m_input_str[m_position + 1] == '/')
    {
      m_position += 2; // Skip "*/"
      m_column_num += 2;
      return;
    }

    if (m_input_str[m_position] == '\n')
    {
      m_line_num++;
      m_column_num = 1;
    }
    else
    {
      m_column_num++;
    }
    m_position++;
  }
}

Token Lexer::parseString()
{
  int start_col = m_column_num;
  m_position++; // Skip opening quote
  m_column_num++;
  size_t start_pos = m_position;

  while (m_position < m_input_str.length() && m_input_str[m_position] != '"')
  {
    m_position++;
    m_column_num++;
  }

  std::string value = m_input_str.substr(start_pos, m_position - start_pos);

  if (m_position < m_input_str.length())
  {
    m_position++; // Skip closing quote
    m_column_num++;
  }

  return {TokenType::String, value, m_line_num, start_col};
}

Token Lexer::parseNumber()
{
  int start_col = m_column_num;
  size_t start_pos = m_position;

  if (m_input_str[m_position] == '-')
  {
    m_position++;
    m_column_num++;
  }

  while (m_position < m_input_str.length() &&
         (isdigit(m_input_str[m_position]) || m_input_str[m_position] == '.'))
  {
    m_position++;
    m_column_num++;
  }
  std::string value = m_input_str.substr(start_pos, m_position - start_pos);
  return {TokenType::Number, value, m_line_num, start_col};
}

Token Lexer::parseIdentifier()
{
  int start_col = m_column_num;
  size_t start_pos = m_position;
  while (m_position < m_input_str.length() &&
         (isalnum(m_input_str[m_position]) || m_input_str[m_position] == '_' ||
          m_input_str[m_position] == '.'))
  {
    m_position++;
    m_column_num++;
  }
  std::string value = m_input_str.substr(start_pos, m_position - start_pos);

  if (value == "true" || value == "false")
  {
    return {TokenType::Boolean, value, m_line_num, start_col};
  }

  return {TokenType::Identifier, value, m_line_num, start_col};
}
} // namespace YINI