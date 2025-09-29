#include "YINI/Lexer.hpp"
#include <cctype>

namespace YINI
{
Lexer::Lexer(const std::string &input)
    : inputText(input), current_position(0), current_line(1), current_column(1)
{
}

Token Lexer::getNextToken()
{
  skipWhitespace();

  if (current_position >= inputText.length())
  {
    return {TokenType::Eof, "", current_line, current_column};
  }

  char current_char = inputText[current_position];

  if (current_char == '/')
  {
    if (current_position + 1 < inputText.length())
    {
      if (inputText[current_position + 1] == '/')
      {
        skipComment();
        return getNextToken();
      }
      else if (inputText[current_position + 1] == '*')
      {
        skipBlockComment();
        return getNextToken();
      }
    }
  }

  if (current_char == '[')
  {
    current_position++;
    current_column++;
    return {TokenType::LeftBracket, "[", current_line, current_column - 1};
  }

  if (current_char == ']')
  {
    current_position++;
    current_column++;
    return {TokenType::RightBracket, "]", current_line, current_column - 1};
  }

  if (current_char == ',')
  {
    current_position++;
    current_column++;
    return {TokenType::Comma, ",", current_line, current_column - 1};
  }

  if (current_char == ':')
  {
    current_position++;
    current_column++;
    return {TokenType::Colon, ":", current_line, current_column - 1};
  }

  if (current_char == '=')
  {
    current_position++;
    current_column++;
    return {TokenType::Equals, "=", current_line, current_column - 1};
  }

  if (current_char == '+' && current_position + 1 < inputText.length() &&
      inputText[current_position + 1] == '=')
  {
    current_position += 2;
    current_column += 2;
    return {TokenType::PlusEquals, "+=", current_line, current_column - 2};
  }

  if (current_char == '@')
  {
    current_position++;
    current_column++;
    return {TokenType::At, "@", current_line, current_column - 1};
  }

  switch (current_char)
  {
  case '+':
    current_position++;
    current_column++;
    return {TokenType::Plus, "+", current_line, current_column - 1};
  case '-':
    current_position++;
    current_column++;
    return {TokenType::Minus, "-", current_line, current_column - 1};
  case '*':
    current_position++;
    current_column++;
    return {TokenType::Star, "*", current_line, current_column - 1};
  case '/':
    current_position++;
    current_column++;
    return {TokenType::Slash, "/", current_line, current_column - 1};
  case '%':
    current_position++;
    current_column++;
    return {TokenType::Percent, "%", current_line, current_column - 1};
  case '(':
    current_position++;
    current_column++;
    return {TokenType::LeftParen, "(", current_line, current_column - 1};
  case ')':
    current_position++;
    current_column++;
    return {TokenType::RightParen, ")", current_line, current_column - 1};
  case '{':
    current_position++;
    current_column++;
    return {TokenType::LeftBrace, "{", current_line, current_column - 1};
  case '}':
    current_position++;
    current_column++;
    return {TokenType::RightBrace, "}", current_line, current_column - 1};
  }

  if (current_char == '#')
  {
    // Check for hex color like #RRGGBB
    if (current_position + 7 <= inputText.length())
    {
      bool is_hex = true;
      for (int i = 1; i <= 6; ++i)
      {
        if (!isxdigit(inputText[current_position + i]))
        {
          is_hex = false;
          break;
        }
      }
      if (is_hex)
      {
        std::string hex_value = inputText.substr(current_position + 1, 6);
        current_position += 7;
        current_column += 7;
        return {TokenType::HexColor, hex_value, current_line, current_column - 7};
      }
    }
    // Otherwise, it's a directive hash
    current_position++;
    current_column++;
    return {TokenType::Hash, "#", current_line, current_column - 1};
  }

  if (current_char == '"')
  {
    return string();
  }

  if (isdigit(current_char) ||
      (current_char == '-' && current_position + 1 < inputText.length() &&
       isdigit(inputText[current_position + 1])))
  {
    return number();
  }

  if (isalpha(current_char) || current_char == '_')
  {
    return identifier();
  }

  current_position++;
  current_column++;
  return {TokenType::Unknown, std::string(1, current_char), current_line,
          current_column - 1};
}

void Lexer::skipWhitespace()
{
  while (current_position < inputText.length() && isspace(inputText[current_position]))
  {
    if (inputText[current_position] == '\n')
    {
      current_line++;
      current_column = 1;
    }
    else
    {
      current_column++;
    }
    current_position++;
  }
}

void Lexer::skipComment()
{
  while (current_position < inputText.length() && inputText[current_position] != '\n')
  {
    current_position++;
    current_column++;
  }
}

void Lexer::skipBlockComment()
{
  current_position += 2; // Skip "/*"
  current_column += 2;

  while (current_position + 1 < inputText.length())
  {
    if (inputText[current_position] == '*' && inputText[current_position + 1] == '/')
    {
      current_position += 2; // Skip "*/"
      current_column += 2;
      return;
    }

    if (inputText[current_position] == '\n')
    {
      current_line++;
      current_column = 1;
    }
    else
    {
      current_column++;
    }
    current_position++;
  }
}

Token Lexer::string()
{
  std::string value;
  int start_col = current_column;
  current_position++; // Skip opening quote
  current_column++;

  while (current_position < inputText.length() && inputText[current_position] != '"')
  {
    value += inputText[current_position];
    current_position++;
    current_column++;
  }

  if (current_position < inputText.length())
  {
    current_position++; // Skip closing quote
    current_column++;
  }

  return {TokenType::String, value, current_line, start_col};
}

Token Lexer::number()
{
  std::string value;
  int start_col = current_column;

  if (inputText[current_position] == '-')
  {
    value += inputText[current_position];
    current_position++;
    current_column++;
  }

  while (current_position < inputText.length() &&
         (isdigit(inputText[current_position]) || inputText[current_position] == '.'))
  {
    value += inputText[current_position];
    current_position++;
    current_column++;
  }
  return {TokenType::Number, value, current_line, start_col};
}

Token Lexer::identifier()
{
  std::string value;
  int start_col = current_column;
  while (current_position < inputText.length() &&
         (isalnum(inputText[current_position]) || inputText[current_position] == '_' ||
          inputText[current_position] == '.'))
  {
    value += inputText[current_position];
    current_position++;
    current_column++;
  }

  if (value == "true" || value == "false")
  {
    return {TokenType::Boolean, value, current_line, start_col};
  }

  return {TokenType::Identifier, value, current_line, start_col};
}
} // namespace YINI