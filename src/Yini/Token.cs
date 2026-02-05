namespace Yini
{
    public enum TokenType
    {
        EndOfFile,
        Identifier,
        StringLiteral,
        NumberLiteral,
        BooleanLiteral, // true, false

        // Symbols
        LBracket, // [
        RBracket, // ]
        LBrace,   // {
        RBrace,   // }
        LParen,   // (
        RParen,   // )
        Colon,    // :
        Comma,    // ,
        Dot,      // .
        Assign,   // =
        PlusAssign, // +=

        // Arithmetic
        Plus,     // +
        Minus,    // -
        Multiply, // *
        Divide,   // /
        Modulo,   // %

        // References
        At,       // @
        Dollar,   // $

        // Special
        Hash,     // # (for colors or #define/#include/#schema)
    }

    public class Token
    {
        public TokenType Type { get; }
        public string Value { get; }
        public int Line { get; }
        public int Column { get; }

        public Token(TokenType type, string value, int line, int column)
        {
            Type = type;
            Value = value;
            Line = line;
            Column = column;
        }

        public override string ToString()
        {
            return $"{Type}('{Value}') at {Line}:{Column}";
        }
    }
}
