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
        Exclamation, // !
        Question, // ?
        Tilde,    // ~
    }

    public class Token
    {
        public TokenType Type { get; }
        public string Value { get; }
        public int Line { get; }
        public int Column { get; }
        public string File { get; set; }

        public Token(TokenType type, string value, int line, int column)
        {
            Type = type;
            Value = value;
            Line = line;
            Column = column;
        }

        public SourceSpan Span => new SourceSpan(File, Line, Column);

        public override string ToString()
        {
            return $"{Type}('{Value}') at {File}:{Line}:{Column}";
        }
    }
}
