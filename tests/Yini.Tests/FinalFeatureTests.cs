using System.Collections.Generic;
using Xunit;
using Yini;
using Yini.CLI;
using Yini.Model;

namespace Yini.Tests
{
    public class MockLocProvider : ILocalizationProvider
    {
        public string GetString(string key) => key == "hello" ? "Bonjour" : key;
    }

    public class FinalFeatureTests
    {
        [Fact]
        public void TestIncrementalBuild()
        {
            // Just test BuildCache logic
            var cache = new BuildCache();
            string file = "test.yini";
            string content = "key = 1";

            Assert.False(cache.IsUpToDate(file, content));
            cache.Update(file, content);
            Assert.True(cache.IsUpToDate(file, content));
            Assert.False(cache.IsUpToDate(file, "key = 2"));
        }

        [Fact]
        public void TestCodeGeneration()
        {
            var src = @"
[#schema]
[Config]
val = !, int
";
            var compiler = new Compiler();
            var doc = compiler.Compile(src);
            var gen = new CodeGenerator();
            string code = gen.GenerateCSharp(doc, "MyGame", "Settings");

            Assert.Contains("namespace MyGame", code);
            Assert.Contains("public class Settings", code);
            Assert.Contains("public class ConfigConfig", code);
            Assert.Contains("public int val { get; set; }", code);
        }

        [Fact]
        public void TestLocalization()
        {
            var compiler = new Compiler(null, new MockLocProvider());
            var src = @"
[UI]
title = @i18n:hello
btn = @i18n:ok
";
            var doc = compiler.Compile(src);
            var section = doc.Sections["UI"];

            Assert.Equal("Bonjour", ((YiniString)section.Properties["title"]).Value);
            // "ok" returns "ok" (mock logic)
            Assert.Equal("ok", ((YiniString)section.Properties["btn"]).Value);
        }
    }
}
