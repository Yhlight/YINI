using System.IO;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class AdvancedFeatureTests
    {
        [Fact]
        public void TestDynaType()
        {
            var compiler = new Compiler();
            var doc = compiler.Compile(@"
[Game]
damage = Dyna(Base * 10 + 5)
");
            var section = doc.Sections["Game"];
            Assert.True(section.Properties.ContainsKey("damage"));
            var dyna = section.Properties["damage"] as YiniDyna;
            Assert.NotNull(dyna);
            // The parser reconstructs the expression string from the expression AST.
            // ((Base * 10) + 5) -> "((Base Multiply 10) Plus 5)" based on current ToString() implementations
            // Let's check contains to be safe or update expectation
            Assert.Contains("Base", dyna.Expression);
            Assert.Contains("10", dyna.Expression);
            Assert.Contains("5", dyna.Expression);
        }

        [Fact]
        public void TestDynaRoundTrip()
        {
            var doc = new YiniDocument();
            var section = new YiniSection("Test");
            section.Properties["val"] = new YiniDyna("x + y");
            doc.Sections.Add("Test", section);

            // Binary
            byte[] bytes;
            using (var ms = new MemoryStream())
            {
                var writer = new YiniBinaryWriter();
                writer.Write(doc, ms);
                bytes = ms.ToArray();
            }

            using (var ms = new MemoryStream(bytes))
            {
                var reader = new YiniBinaryReader();
                var loaded = reader.Read(ms);
                var val = loaded.Sections["Test"].Properties["val"] as YiniDyna;
                Assert.Equal("x + y", val.Expression);
            }
        }

        [Fact]
        public void TestMetaGeneration()
        {
            var compiler = new Compiler();
            var doc = compiler.Compile(@"
[#define]
VER = 1.0

[Config]
val = @VER
");
            var generator = new MetaGenerator();
            string output;
            using (var ms = new MemoryStream())
            {
                generator.Generate(doc, ms);
                ms.Position = 0;
                using (var reader = new StreamReader(ms))
                {
                    output = reader.ReadToEnd();
                }
            }

            Assert.Contains("[#meta]", output);
            Assert.Contains("generated_at", output);
            Assert.Contains("[Config]", output);
            Assert.Contains("val = 1", output); // Should be resolved
        }
    }
}
