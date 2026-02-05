using System;
using System.Collections.Generic;

namespace Yini
{
    // A simplified, allocation-free token struct
    public readonly ref struct SpanToken
    {
        public readonly TokenType Type;
        public readonly ReadOnlySpan<char> Value;
        public readonly int Line;
        public readonly int Column;

        public SpanToken(TokenType type, ReadOnlySpan<char> value, int line, int column)
        {
            Type = type;
            Value = value;
            Line = line;
            Column = column;
        }
    }

    public ref struct LexerFast
    {
        private readonly ReadOnlySpan<char> _input;
        private int _position;
        private int _line;
        private int _column;

        public LexerFast(ReadOnlySpan<char> input)
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

        public SpanToken NextToken()
        {
            while (_position < _input.Length)
            {
                char current = Current;

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
                        while (Current != '\n' && Current != '\0') Advance();
                        continue;
                    }
                    else if (Peek() == '*')
                    {
                        Advance(); Advance();
                        while (!(Current == '*' && Peek() == '/') && Current != '\0') Advance();
                        Advance(); Advance();
                        continue;
                    }
                }

                int startPos = _position;
                int startLine = _line;
                int startCol = _column;

                if (char.IsDigit(current))
                {
                    return ReadNumber(startPos, startLine, startCol);
                }
                else if (current == '.' && char.IsDigit(Peek()))
                {
                    return ReadNumber(startPos, startLine, startCol);
                }
                else if (char.IsLetter(current) || current == '_')
                {
                    return ReadIdentifier(startPos, startLine, startCol);
                }
                else if (current == '"')
                {
                    return ReadString(startPos, startLine, startCol);
                }

                // Symbols
                Advance();
                ReadOnlySpan<char> val = _input.Slice(startPos, 1);
                switch (current)
                {
                    case '[': return new SpanToken(TokenType.LBracket, val, startLine, startCol);
                    case ']': return new SpanToken(TokenType.RBracket, val, startLine, startCol);
                    case '{': return new SpanToken(TokenType.LBrace, val, startLine, startCol);
                    case '}': return new SpanToken(TokenType.RBrace, val, startLine, startCol);
                    case '(': return new SpanToken(TokenType.LParen, val, startLine, startCol);
                    case ')': return new SpanToken(TokenType.RParen, val, startLine, startCol);
                    case ':': return new SpanToken(TokenType.Colon, val, startLine, startCol);
                    case ',': return new SpanToken(TokenType.Comma, val, startLine, startCol);
                    case '.': return new SpanToken(TokenType.Dot, val, startLine, startCol);
                    case '=': return new SpanToken(TokenType.Assign, val, startLine, startCol);
                    case '+':
                        if (Current == '=')
                        {
                            Advance();
                            return new SpanToken(TokenType.PlusAssign, _input.Slice(startPos, 2), startLine, startCol);
                        }
                        return new SpanToken(TokenType.Plus, val, startLine, startCol);
                    case '-': return new SpanToken(TokenType.Minus, val, startLine, startCol);
                    case '*': return new SpanToken(TokenType.Multiply, val, startLine, startCol);
                    case '/': return new SpanToken(TokenType.Divide, val, startLine, startCol);
                    case '%': return new SpanToken(TokenType.Modulo, val, startLine, startCol);
                    case '@': return new SpanToken(TokenType.At, val, startLine, startCol);
                    case '$': return new SpanToken(TokenType.Dollar, val, startLine, startCol);
                    case '#': return new SpanToken(TokenType.Hash, val, startLine, startCol);
                    case '!': return new SpanToken(TokenType.Exclamation, val, startLine, startCol);
                    case '?': return new SpanToken(TokenType.Question, val, startLine, startCol);
                    case '~': return new SpanToken(TokenType.Tilde, val, startLine, startCol);
                    default: break; // Error or unknown
                }
            }
            return new SpanToken(TokenType.EndOfFile, ReadOnlySpan<char>.Empty, _line, _column);
        }

        private SpanToken ReadNumber(int startPos, int line, int col)
        {
            while (char.IsDigit(Current) || Current == '.')
            {
                Advance();
            }
            return new SpanToken(TokenType.NumberLiteral, _input.Slice(startPos, _position - startPos), line, col);
        }

        private SpanToken ReadIdentifier(int startPos, int line, int col)
        {
            while (char.IsLetterOrDigit(Current) || Current == '_')
            {
                Advance();
            }
            var val = _input.Slice(startPos, _position - startPos);
            // Check keywords?
            if (val.SequenceEqual("true") || val.SequenceEqual("false"))
                return new SpanToken(TokenType.BooleanLiteral, val, line, col);

            return new SpanToken(TokenType.Identifier, val, line, col);
        }

        private SpanToken ReadString(int startPos, int line, int col)
        {
            Advance(); // quote
            while (Current != '"' && Current != '\0')
            {
                if (Current == '\\') Advance();
                Advance();
            }
            if (Current == '"') Advance();
            // Value includes quotes for now, stripping usually happens later or we can slice inside
            return new SpanToken(TokenType.StringLiteral, _input.Slice(startPos + 1, _position - startPos - 2), line, col);
        }
    }
}
