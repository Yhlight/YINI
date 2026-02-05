using System.IO;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class OptimizationTests
    {
        [Fact]
        public void TestStructParsing()
        {
            var compiler = new Compiler();
            var doc = compiler.Compile(@"
[Config]
st = {k: 1, v: 2}
mp = {k: 1, v: 2,}
");
            var section = doc.Sections["Config"];
            var st = section.Properties["st"];
            var mp = section.Properties["mp"];

            Assert.IsType<YiniStruct>(st);
            Assert.IsType<YiniMap>(mp);

            Assert.Equal(1, ((YiniInteger)((YiniStruct)st).Fields["k"]).Value);
            Assert.Equal(1, ((YiniInteger)((YiniMap)mp).Items["k"]).Value);
        }

        [Fact]
        public void TestStructRoundTrip()
        {
            var doc = new YiniDocument();
            var section = new YiniSection("Test");
            var str = new YiniStruct();
            str.Fields["x"] = new YiniInteger(10);
            section.Properties["val"] = str;
            doc.Sections.Add("Test", section);

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
                var val = loaded.Sections["Test"].Properties["val"];
                Assert.IsType<YiniStruct>(val);
                Assert.Equal(10, ((YiniInteger)((YiniStruct)val).Fields["x"]).Value);
            }
        }
    }
}
