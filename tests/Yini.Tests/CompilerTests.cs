using Xunit;
using Yini;
using Yini.Model;
using System.Linq;

namespace Yini.Tests
{
    public class CompilerTests
    {
        [Fact]
        public void TestBasicConfig()
        {
            string src = @"
[Config]
key1 = 123
key2 = ""hello""
key3 = true
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            Assert.True(doc.Sections.ContainsKey("Config"));
            var config = doc.Sections["Config"];
            Assert.Equal(123, ((YiniInteger)config.Properties["key1"]).Value);
            Assert.Equal("hello", ((YiniString)config.Properties["key2"]).Value);
            Assert.True(((YiniBoolean)config.Properties["key3"]).Value);
        }

        [Fact]
        public void TestInheritance()
        {
            string src = @"
[Parent]
val = 1

[Child] : Parent
val2 = 2
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            var child = doc.Sections["Child"];
            Assert.True(child.Properties.ContainsKey("val"));
            Assert.Equal(1, ((YiniInteger)child.Properties["val"]).Value);
            Assert.Equal(2, ((YiniInteger)child.Properties["val2"]).Value);
        }

        [Fact]
        public void TestArithmeticAndMacro()
        {
            string src = @"
[#define]
base = 100

[Config]
val = @base + 50
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            var config = doc.Sections["Config"];
            Assert.Equal(150, ((YiniInteger)config.Properties["val"]).Value);
        }

        [Fact]
        public void TestComplexTypes()
        {
            string src = @"
[Visual]
color = #FF0000
coord = Coord(10.5, 20.5)
list = [1, 2, 3]
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            var visual = doc.Sections["Visual"];
            var color = (YiniColor)visual.Properties["color"];
            Assert.Equal(255, color.R);
            Assert.Equal(0, color.G);

            var coord = (YiniCoord)visual.Properties["coord"];
            Assert.Equal(10.5f, coord.X);

            var list = (YiniArray)visual.Properties["list"];
            Assert.Equal(3, list.Items.Count);
        }

        [Fact]
        public void TestCrossSectionRef()
        {
             string src = @"
[A]
val = 10

[B]
val = @{A.val} * 2
";
             var compiler = new Compiler();
             var doc = compiler.Compile(src);

             var b = doc.Sections["B"];
             Assert.Equal(20, ((YiniInteger)b.Properties["val"]).Value);
        }

        [Fact]
        public void TestRegistry()
        {
            string src = @"
[Reg]
+= 1
+= 2
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);

            var reg = doc.Sections["Reg"];
            Assert.Equal(2, reg.Registry.Count);
            Assert.Equal(1, ((YiniInteger)reg.Registry[0]).Value);
            Assert.Equal(2, ((YiniInteger)reg.Registry[1]).Value);
        }
    }
}
