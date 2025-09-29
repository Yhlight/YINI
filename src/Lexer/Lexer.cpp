#include "YINI/Lexer.hpp"
#include <cctype>

namespace YINI
{
Lexer::Lexer(const std::string &input)
    : inputStr(input), position(0), line_num(1), column_num(1)
{
}

Token Lexer::getNextToken()
{
  skipWhitespace();

  if (position >= inputStr.length())
  {
    return {TokenType::Eof, "", line_num, column_num};
  }

  char current_char = inputStr[position];

  if (current_char == '/')
  {
    if (position + 1 < inputStr.length())
    {
      if (inputStr[position + 1] == '/')
      {
        skipComment();
        return getNextToken();
      }
      else if (inputStr[position + 1] == '*')
      {
        skipBlockComment();
        return getNextToken();
      }
    }
  }

  if (current_char == '[')
  {
    position++;
    column_num++;
    return {TokenType::LeftBracket, "[", line_num, column_num - 1};
  }

  if (current_char == ']')
  {
    position++;
    column_num++;
    return {TokenType::RightBracket, "]", line_num, column_num - 1};
  }

  if (current_char == ',')
  {
    position++;
    column_num++;
    return {TokenType::Comma, ",", line_num, column_num - 1};
  }

  if (current_char == ':')
  {
    position++;
    column_num++;
    return {TokenType::Colon, ":", line_num, column_num - 1};
  }

  if (current_char == '=')
  {
    position++;
    column_num++;
    return {TokenType::Equals, "=", line_num, column_num - 1};
  }

  if (current_char == '+' && position + 1 < inputStr.length() &&
      inputStr[position + 1] == '=')
  {
    position += 2;
    column_num += 2;
    return {TokenType::PlusEquals, "+=", line_num, column_num - 2};
  }

  if (current_char == '@')
  {
    position++;
    column_num++;
    return {TokenType::At, "@", line_num, column_num - 1};
  }

  switch (current_char)
  {
  case '+':
    position++;
    column_num++;
    return {TokenType::Plus, "+", line_num, column_num - 1};
  case '-':
    position++;
    column_num++;
    return {TokenType::Minus, "-", line_num, column_num - 1};
  case '*':
    position++;
    column_num++;
    return {TokenType::Star, "*", line_num, column_num - 1};
  case '/':
    position++;
    column_num++;
    return {TokenType::Slash, "/", line_num, column_num - 1};
  case '%':
    position++;
    column_num++;
    return {TokenType::Percent, "%", line_num, column_num - 1};
  case '(':
    position++;
    column_num++;
    return {TokenType::LeftParen, "(", line_num, column_num - 1};
  case ')':
    position++;
    column_num++;
    return {TokenType::RightParen, ")", line_num, column_num - 1};
  case '{':
    position++;
    column_num++;
    return {TokenType::LeftBrace, "{", line_num, column_num - 1};
  case '}':
    position++;
    column_num++;
    return {TokenType::RightBrace, "}", line_num, column_num - 1};
  }

  if (current_char == '#')
  {
    // Check for hex color like #RRGGBB
    if (position + 7 <= inputStr.length())
    {
      bool is_hex = true;
      for (int i = 1; i <= 6; ++i)
      {
        if (!isxdigit(inputStr[position + i]))
        {
          is_hex = false;
          break;
        }
      }
      if (is_hex)
      {
        std::string hex_value = inputStr.substr(position + 1, 6);
        position += 7;
        column_num += 7;
        return {TokenType::HexColor, hex_value, line_num, column_num - 7};
      }
    }
    // Otherwise, it's a directive hash
    position++;
    column_num++;
    return {TokenType::Hash, "#", line_num, column_num - 1};
  }

  if (current_char == '"')
  {
    return parseString();
  }

  if (isdigit(current_char) ||
      (current_char == '-' && position + 1 < inputStr.length() &&
       isdigit(inputStr[position + 1])))
  {
    return parseNumber();
  }

  if (isalpha(current_char) || current_char == '_')
  {
    return parseIdentifier();
  }

  position++;
  column_num++;
  return {TokenType::Unknown, std::string(1, current_char), line_num,
          column_num - 1};
}

void Lexer::skipWhitespace()
{
  while (position < inputStr.length() && isspace(inputStr[position]))
  {
    if (inputStr[position] == '\n')
    {
      line_num++;
      column_num = 1;
    }
    else
    {
      column_num++;
    }
    position++;
  }
}

void Lexer::skipComment()
{
  while (position < inputStr.length() && inputStr[position] != '\n')
  {
    position++;
    column_num++;
  }
}

void Lexer::skipBlockComment()
{
  position += 2; // Skip "/*"
  column_num += 2;

  while (position + 1 < inputStr.length())
  {
    if (inputStr[position] == '*' && inputStr[position + 1] == '/')
    {
      position += 2; // Skip "*/"
      column_num += 2;
      return;
    }

    if (inputStr[position] == '\n')
    {
      line_num++;
      column_num = 1;
    }
    else
    {
      column_num++;
    }
    position++;
  }
}

Token Lexer::parseString()
{
  std::string value;
  int start_col = column_num;
  position++; // Skip opening quote
  column_num++;

  while (position < inputStr.length() && inputStr[position] != '"')
  {
    value += inputStr[position];
    position++;
    column_num++;
  }

  if (position < inputStr.length())
  {
    position++; // Skip closing quote
    column_num++;
  }

  return {TokenType::String, value, line_num, start_col};
}

Token Lexer::parseNumber()
{
  std::string value;
  int start_col = column_num;

  if (inputStr[position] == '-')
  {
    value += inputStr[position];
    position++;
    column_num++;
  }

  while (position < inputStr.length() &&
         (isdigit(inputStr[position]) || inputStr[position] == '.'))
  {
    value += inputStr[position];
    position++;
    column_num++;
  }
  return {TokenType::Number, value, line_num, start_col};
}

Token Lexer::parseIdentifier()
{
  std::string value;
  int start_col = column_num;
  while (position < inputStr.length() &&
         (isalnum(inputStr[position]) || inputStr[position] == '_' ||
          inputStr[position] == '.'))
  {
    value += inputStr[position];
    position++;
    column_num++;
  }

  if (value == "true" || value == "false")
  {
    return {TokenType::Boolean, value, line_num, start_col};
  }

  return {TokenType::Identifier, value, line_num, start_col};
}
} // namespace YINI