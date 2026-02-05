using System;

namespace Yini
{
    public class SourceSpan
    {
        public string File { get; }
        public int Line { get; }
        public int Column { get; }

        public SourceSpan(string file, int line, int column)
        {
            File = file;
            Line = line;
            Column = column;
        }

        public override string ToString() => $"{File}:{Line}:{Column}";
    }

    public class YiniException : Exception
    {
        public SourceSpan Span { get; }

        public YiniException(string message, SourceSpan span)
            : base($"{message} at {span}")
        {
            Span = span;
        }
    }
}
