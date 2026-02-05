using System;
using Xunit;
using Yini;
using Yini.Bytecode;
using Yini.Model;

namespace Yini.Tests
{
    public class SuggestionFeatureTests
    {
        [Fact]
        public void TestBytecodeVM()
        {
            // 10 + 20 * 3 = 70
            var expr = new YiniBinaryExpression(
                new YiniInteger(10),
                TokenType.Plus,
                new YiniBinaryExpression(
                    new YiniInteger(20),
                    TokenType.Multiply,
                    new YiniInteger(3)
                )
            );

            var compiler = new BytecodeCompiler();
            byte[] code = compiler.Compile(expr);

            var vm = new BytecodeVM(null); // No context needed for constants
            var result = vm.Run(code);

            Assert.IsType<YiniInteger>(result);
            Assert.Equal(70, ((YiniInteger)result).Value);
        }

        [Fact]
        public void TestPatcher()
        {
            var compiler = new Compiler();
            var baseDoc = compiler.Compile(@"
[Config]
Mode = ""Easy""
Lives = 3
");
            var patchDoc = compiler.Compile(@"
[Config]
Lives = 99
Cheat = true
");

            ConfigPatcher.ApplyPatch(baseDoc, patchDoc);

            var section = baseDoc.Sections["Config"];
            Assert.Equal("Easy", ((YiniString)section.Properties["Mode"]).Value);
            Assert.Equal(99, ((YiniInteger)section.Properties["Lives"]).Value); // Overwritten
            Assert.True(((YiniBoolean)section.Properties["Cheat"]).Value); // Added
        }

        [Fact]
        public void TestCodeGenerator_IncludesLoad()
        {
            var gen = new CodeGenerator();
            var doc = new YiniDocument();
            doc.Schemas["Player"] = new YiniSection("Player");
            // Add a property to schema
            var schemaDef = new YiniSchemaDefinition { TypeName = "int" };
            doc.Schemas["Player"].Properties["Health"] = schemaDef;

            string code = gen.GenerateCSharp(doc, "Game", "Config");

            Assert.Contains("public void Load(YiniDocument doc)", code);
            Assert.Contains("public class PlayerConfig", code);
            Assert.Contains("public int Health { get; set; }", code);
        }
    }
}
