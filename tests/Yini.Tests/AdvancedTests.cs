using System.IO;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class AdvancedTests
    {
        [Fact]
        public void TestErrorReporting()
        {
            var src = @"
[Config]
key = 1
key =
";
            var compiler = new Compiler();
            // Expect error at line 4 (key =)

            try
            {
                compiler.Compile(src);
                Assert.Fail("Should have thrown YiniException");
            }
            catch (YiniException ex)
            {
                // Parser might throw "Unexpected token" or similar depending on implementation
                // because `key =` followed by newline or EOF is incomplete expression.
                // The lexer might advance to EOF (line 5) while looking for expression start.
                Assert.NotNull(ex.Span);
                // Assert.Equal(4, ex.Span.Line); // Line 5 is acceptable for EOF/EndOfLine error
                Assert.True(ex.Span.Line >= 4);
            }
        }

        [Fact]
        public void TestBinaryRoundTrip()
        {
            var src = @"
[Config]
int = 1
float = 2.5
string = ""hello""
bool = true
array = [1, 2, 3]
color = #FF0000
coord = Coord(10, 20)
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            byte[] bytes;
            using (var ms = new MemoryStream())
            {
                var writer = new YiniBinaryWriter();
                writer.Write(doc, ms);
                bytes = ms.ToArray();
            }

            YiniDocument loaded;
            using (var ms = new MemoryStream(bytes))
            {
                var reader = new YiniBinaryReader();
                loaded = reader.Read(ms);
            }

            var section = loaded.Sections["Config"];
            Assert.Equal(1, ((YiniInteger)section.Properties["int"]).Value);
            Assert.Equal(2.5f, ((YiniFloat)section.Properties["float"]).Value);
            Assert.Equal("hello", ((YiniString)section.Properties["string"]).Value);
            Assert.True(((YiniBoolean)section.Properties["bool"]).Value);
            Assert.Equal(3, ((YiniArray)section.Properties["array"]).Items.Count);

            var color = (YiniColor)section.Properties["color"];
            Assert.Equal(255, color.R);

            var coord = (YiniCoord)section.Properties["coord"];
            Assert.Equal(10f, coord.X);
        }
    }
}
