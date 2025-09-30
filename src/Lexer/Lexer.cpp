#include "YINI/Lexer.hpp"
#include <cctype>

namespace YINI
{
Lexer::Lexer(const std::string &input)
    : m_input(input), m_position(0), m_line(1), m_column(1)
{
}

Token Lexer::getNextToken()
{
  skipWhitespace();

  if (m_position >= m_input.length())
  {
    return {TokenType::Eof, "", m_line, m_column};
  }

  char current_char = m_input[m_position];

  if (current_char == '/')
  {
    if (m_position + 1 < m_input.length())
    {
      if (m_input[m_position + 1] == '/')
      {
        skipComment();
        return getNextToken();
      }
      else if (m_input[m_position + 1] == '*')
      {
        skipBlockComment();
        return getNextToken();
      }
    }
  }

  if (current_char == '[')
  {
    m_position++;
    m_column++;
    return {TokenType::LeftBracket, "[", m_line, m_column - 1};
  }

  if (current_char == ']')
  {
    m_position++;
    m_column++;
    return {TokenType::RightBracket, "]", m_line, m_column - 1};
  }

  if (current_char == ',')
  {
    m_position++;
    m_column++;
    return {TokenType::Comma, ",", m_line, m_column - 1};
  }

  if (current_char == ':')
  {
    m_position++;
    m_column++;
    return {TokenType::Colon, ":", m_line, m_column - 1};
  }

  if (current_char == '=')
  {
    m_position++;
    m_column++;
    return {TokenType::Equals, "=", m_line, m_column - 1};
  }

  if (current_char == '+' && m_position + 1 < m_input.length() &&
      m_input[m_position + 1] == '=')
  {
    m_position += 2;
    m_column += 2;
    return {TokenType::PlusEquals, "+=", m_line, m_column - 2};
  }

  if (current_char == '@')
  {
    m_position++;
    m_column++;
    return {TokenType::At, "@", m_line, m_column - 1};
  }

  switch (current_char)
  {
  case '+':
    m_position++;
    m_column++;
    return {TokenType::Plus, "+", m_line, m_column - 1};
  case '-':
    m_position++;
    m_column++;
    return {TokenType::Minus, "-", m_line, m_column - 1};
  case '*':
    m_position++;
    m_column++;
    return {TokenType::Star, "*", m_line, m_column - 1};
  case '/':
    m_position++;
    m_column++;
    return {TokenType::Slash, "/", m_line, m_column - 1};
  case '%':
    m_position++;
    m_column++;
    return {TokenType::Percent, "%", m_line, m_column - 1};
  case '(':
    m_position++;
    m_column++;
    return {TokenType::LeftParen, "(", m_line, m_column - 1};
  case ')':
    m_position++;
    m_column++;
    return {TokenType::RightParen, ")", m_line, m_column - 1};
  case '{':
    m_position++;
    m_column++;
    return {TokenType::LeftBrace, "{", m_line, m_column - 1};
  case '}':
    m_position++;
    m_column++;
    return {TokenType::RightBrace, "}", m_line, m_column - 1};
  }

  if (current_char == '#')
  {
    // Check for hex color like #RRGGBB
    if (m_position + 7 <= m_input.length())
    {
      bool is_hex = true;
      for (int i = 1; i <= 6; ++i)
      {
        if (!isxdigit(m_input[m_position + i]))
        {
          is_hex = false;
          break;
        }
      }
      if (is_hex)
      {
        std::string hex_value = m_input.substr(m_position + 1, 6);
        m_position += 7;
        m_column += 7;
        return {TokenType::HexColor, hex_value, m_line, m_column - 7};
      }
    }
    // Otherwise, it's a directive hash
    m_position++;
    m_column++;
    return {TokenType::Hash, "#", m_line, m_column - 1};
  }

  if (current_char == '"')
  {
    return string();
  }

  if (isdigit(current_char) ||
      (current_char == '-' && m_position + 1 < m_input.length() &&
       isdigit(m_input[m_position + 1])))
  {
    return number();
  }

  if (isalpha(current_char) || current_char == '_')
  {
    return identifier();
  }

  m_position++;
  m_column++;
  return {TokenType::Unknown, std::string(1, current_char), m_line,
          m_column - 1};
}

void Lexer::skipWhitespace()
{
  while (m_position < m_input.length() && isspace(m_input[m_position]))
  {
    if (m_input[m_position] == '\n')
    {
      m_line++;
      m_column = 1;
    }
    else
    {
      m_column++;
    }
    m_position++;
  }
}

void Lexer::skipComment()
{
  while (m_position < m_input.length() && m_input[m_position] != '\n')
  {
    m_position++;
    m_column++;
  }
}

void Lexer::skipBlockComment()
{
  m_position += 2; // Skip "/*"
  m_column += 2;

  while (m_position + 1 < m_input.length())
  {
    if (m_input[m_position] == '*' && m_input[m_position + 1] == '/')
    {
      m_position += 2; // Skip "*/"
      m_column += 2;
      return;
    }

    if (m_input[m_position] == '\n')
    {
      m_line++;
      m_column = 1;
    }
    else
    {
      m_column++;
    }
    m_position++;
  }
}

Token Lexer::string()
{
  std::string value;
  int start_col = m_column;
  m_position++; // Skip opening quote
  m_column++;

  while (m_position < m_input.length() && m_input[m_position] != '"')
  {
    value += m_input[m_position];
    m_position++;
    m_column++;
  }

  if (m_position < m_input.length())
  {
    m_position++; // Skip closing quote
    m_column++;
  }

  return {TokenType::String, value, m_line, start_col};
}

Token Lexer::number()
{
  std::string value;
  int start_col = m_column;

  if (m_input[m_position] == '-')
  {
    value += m_input[m_position];
    m_position++;
    m_column++;
  }

  while (m_position < m_input.length() &&
         (isdigit(m_input[m_position]) || m_input[m_position] == '.'))
  {
    value += m_input[m_position];
    m_position++;
    m_column++;
  }
  return {TokenType::Number, value, m_line, start_col};
}

Token Lexer::identifier()
{
  std::string value;
  int start_col = m_column;
  while (m_position < m_input.length() &&
         (isalnum(m_input[m_position]) || m_input[m_position] == '_' ||
          m_input[m_position] == '.'))
  {
    value += m_input[m_position];
    m_position++;
    m_column++;
  }

  if (value == "true" || value == "false")
  {
    return {TokenType::Boolean, value, m_line, start_col};
  }

  return {TokenType::Identifier, value, m_line, start_col};
}
} // namespace YINI