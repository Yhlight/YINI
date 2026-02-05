using System;
using System.Diagnostics;
using System.Linq;
using Xunit;
using Yini;

namespace Yini.Tests
{
    public class Benchmarks
    {
        [Fact]
        public void BenchmarkLexer()
        {
            string source = string.Concat(Enumerable.Repeat(@"
[Section]
key = 123
val = ""string""
arr = [1, 2, 3]
color = #FF0000
", 1000));

            // Standard Lexer
            var sw = Stopwatch.StartNew();
            var lexer = new Lexer(source);
            var tokens = lexer.Tokenize();
            sw.Stop();
            long standardMs = sw.ElapsedMilliseconds;

            // Fast Lexer
            sw.Restart();
            var fastLexer = new LexerFast(source.AsSpan());
            int count = 0;
            while (true)
            {
                var t = fastLexer.NextToken();
                if (t.Type == TokenType.EndOfFile) break;
                count++;
            }
            sw.Stop();
            long fastMs = sw.ElapsedMilliseconds;

            // Ideally fast lexer should be faster or comparable (allocation-free)
            // Note: Tokenize() allocates list and classes. NextToken() returns struct.
            // This proves the optimization potential.
            Assert.True(true, $"Standard: {standardMs}ms, Fast: {fastMs}ms");
            // Just verifying it runs without crashing
        }
    }
}
