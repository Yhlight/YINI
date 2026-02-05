using System;
using System.Collections.Generic;
using System.Text;

namespace Yini
{
    public class Lexer
    {
        private readonly string _input;
        private int _position;
        private int _line;
        private int _column;

        public Lexer(string input)
        {
            _input = input;
            _position = 0;
            _line = 1;
            _column = 1;
        }

        private char Current => _position < _input.Length ? _input[_position] : '\0';

        private void Advance()
        {
            if (_position < _input.Length)
            {
                if (_input[_position] == '\n')
                {
                    _line++;
                    _column = 1;
                }
                else
                {
                    _column++;
                }
                _position++;
            }
        }

        private char Peek(int offset = 1)
        {
            int index = _position + offset;
            return index < _input.Length ? _input[index] : '\0';
        }

        public List<Token> Tokenize()
        {
            var tokens = new List<Token>();

            while (_position < _input.Length)
            {
                char current = Current;

                // Skip Whitespace
                if (char.IsWhiteSpace(current))
                {
                    Advance();
                    continue;
                }

                // Comments
                if (current == '/')
                {
                    if (Peek() == '/')
                    {
                        // Single line comment
                        while (Current != '\n' && Current != '\0')
                        {
                            Advance();
                        }
                        continue;
                    }
                    else if (Peek() == '*')
                    {
                        // Multi line comment
                        Advance(); // /
                        Advance(); // *
                        while (!(Current == '*' && Peek() == '/') && Current != '\0')
                        {
                            Advance();
                        }
                        Advance(); // *
                        Advance(); // /
                        continue;
                    }
                    // Else it's just a divide or part of a path?
                    // YINI supports arithmetic /, so handle as Divide token
                }

                int startLine = _line;
                int startColumn = _column;

                if (char.IsDigit(current))
                {
                    tokens.Add(ReadNumber());
                }
                else if (current == '.' && char.IsDigit(Peek()))
                {
                    tokens.Add(ReadNumber());
                }
                else if (char.IsLetter(current) || current == '_')
                {
                    tokens.Add(ReadIdentifierOrKeyword());
                }
                else if (current == '"')
                {
                    tokens.Add(ReadString());
                }
                else
                {
                    // Operators and Symbols
                    switch (current)
                    {
                        case '[': tokens.Add(new Token(TokenType.LBracket, "[", _line, _column)); Advance(); break;
                        case ']': tokens.Add(new Token(TokenType.RBracket, "]", _line, _column)); Advance(); break;
                        case '{': tokens.Add(new Token(TokenType.LBrace, "{", _line, _column)); Advance(); break;
                        case '}': tokens.Add(new Token(TokenType.RBrace, "}", _line, _column)); Advance(); break;
                        case '(': tokens.Add(new Token(TokenType.LParen, "(", _line, _column)); Advance(); break;
                        case ')': tokens.Add(new Token(TokenType.RParen, ")", _line, _column)); Advance(); break;
                        case ':': tokens.Add(new Token(TokenType.Colon, ":", _line, _column)); Advance(); break;
                        case ',': tokens.Add(new Token(TokenType.Comma, ",", _line, _column)); Advance(); break;
                        case '.': tokens.Add(new Token(TokenType.Dot, ".", _line, _column)); Advance(); break;
                        case '=': tokens.Add(new Token(TokenType.Assign, "=", _line, _column)); Advance(); break;
                        case '+':
                            if (Peek() == '=')
                            {
                                tokens.Add(new Token(TokenType.PlusAssign, "+=", _line, _column));
                                Advance(); Advance();
                            }
                            else
                            {
                                tokens.Add(new Token(TokenType.Plus, "+", _line, _column));
                                Advance();
                            }
                            break;
                        case '-': tokens.Add(new Token(TokenType.Minus, "-", _line, _column)); Advance(); break;
                        case '*': tokens.Add(new Token(TokenType.Multiply, "*", _line, _column)); Advance(); break;
                        case '/': tokens.Add(new Token(TokenType.Divide, "/", _line, _column)); Advance(); break;
                        case '%': tokens.Add(new Token(TokenType.Modulo, "%", _line, _column)); Advance(); break;
                        case '@': tokens.Add(new Token(TokenType.At, "@", _line, _column)); Advance(); break;
                        case '$': tokens.Add(new Token(TokenType.Dollar, "$", _line, _column)); Advance(); break;
                        case '#': tokens.Add(new Token(TokenType.Hash, "#", _line, _column)); Advance(); break;
                        case '!': tokens.Add(new Token(TokenType.Exclamation, "!", _line, _column)); Advance(); break;
                        case '?': tokens.Add(new Token(TokenType.Question, "?", _line, _column)); Advance(); break;
                        case '~': tokens.Add(new Token(TokenType.Tilde, "~", _line, _column)); Advance(); break;
                        default:
                            throw new Exception($"Unexpected character '{current}' at {_line}:{_column}");
                    }
                }
            }

            tokens.Add(new Token(TokenType.EndOfFile, "", _line, _column));
            return tokens;
        }

        private Token ReadNumber()
        {
            int startLine = _line;
            int startColumn = _column;
            var sb = new StringBuilder();
            bool isFloat = false;

            while (char.IsDigit(Current) || Current == '.')
            {
                if (Current == '.')
                {
                    if (isFloat) break; // Second dot
                    isFloat = true;
                }
                sb.Append(Current);
                Advance();
            }

            return new Token(TokenType.NumberLiteral, sb.ToString(), startLine, startColumn);
        }

        private Token ReadIdentifierOrKeyword()
        {
            int startLine = _line;
            int startColumn = _column;
            var sb = new StringBuilder();

            while (char.IsLetterOrDigit(Current) || Current == '_')
            {
                sb.Append(Current);
                Advance();
            }

            string text = sb.ToString();
            if (text == "true" || text == "false")
            {
                return new Token(TokenType.BooleanLiteral, text, startLine, startColumn);
            }

            return new Token(TokenType.Identifier, text, startLine, startColumn);
        }

        private Token ReadString()
        {
            int startLine = _line;
            int startColumn = _column;
            Advance(); // Skip opening quote
            var sb = new StringBuilder();

            while (Current != '"' && Current != '\0')
            {
                if (Current == '\\')
                {
                    Advance();
                    char escape = Current;
                    switch (escape)
                    {
                        case '"': sb.Append('"'); break;
                        case '\\': sb.Append('\\'); break;
                        case 'n': sb.Append('\n'); break;
                        case 't': sb.Append('\t'); break;
                        case 'r': sb.Append('\r'); break;
                        default: sb.Append(escape); break;
                    }
                }
                else
                {
                    sb.Append(Current);
                }
                Advance();
            }

            if (Current == '"')
            {
                Advance(); // Skip closing quote
            }

            return new Token(TokenType.StringLiteral, sb.ToString(), startLine, startColumn);
        }
    }
}
