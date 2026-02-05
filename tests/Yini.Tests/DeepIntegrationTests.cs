using System.Collections.Generic;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class MockFileLoader : IFileLoader
    {
        public Dictionary<string, string> Files { get; } = new Dictionary<string, string>();

        public bool Exists(string path) => Files.ContainsKey(path);

        public string LoadFile(string path) => Files[path];
    }

    public class DeepIntegrationTests
    {
        [Fact]
        public void TestIncludes()
        {
            var loader = new MockFileLoader();
            loader.Files["base.yini"] = @"
[Base]
x = 10
y = 20
";
            loader.Files["sub.yini"] = @"
[#include]
+= base.yini

[Base]
x = 30
";

            // sub includes base.
            // Order: Load base. Merge base. Merge sub (overrides base).
            // Result: x=30 (sub), y=20 (base).

            var compiler = new Compiler(loader);
            var doc = compiler.Compile(@"
[#include]
+= sub.yini
");
            // Main includes sub. Sub includes base.
            // Recursive merge.

            Assert.True(doc.Sections.ContainsKey("Base"));
            var section = doc.Sections["Base"];
            Assert.Equal(30, ((YiniInteger)section.Properties["x"]).Value);
            Assert.Equal(20, ((YiniInteger)section.Properties["y"]).Value);
        }

        [Fact]
        public void TestValidator_Required()
        {
            var loader = new MockFileLoader();
            loader.Files["schema.yini"] = @"
[#schema]
[Config]
req = !, int
";
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(@"
[#include]
+= schema.yini

[Config]
// Missing req
");
            var validator = new Validator();

            var ex = Assert.Throws<ValidationException>(() => validator.Validate(doc));
            Assert.Contains("Missing required property: req", ex.Message);
        }

        [Fact]
        public void TestValidator_Default()
        {
            var loader = new MockFileLoader();
            loader.Files["schema.yini"] = @"
[#schema]
[Config]
opt = ?, int, =100
";
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(@"
[#include]
+= schema.yini

[Config]
// Missing opt
");
            var validator = new Validator();
            validator.Validate(doc);

            var section = doc.Sections["Config"];
            Assert.True(section.Properties.ContainsKey("opt"));
            Assert.Equal(100, ((YiniInteger)section.Properties["opt"]).Value);
        }

        [Fact]
        public void TestValidator_Range()
        {
            var loader = new MockFileLoader();
            loader.Files["schema.yini"] = @"
[#schema]
[Config]
val = !, int, min=10, max=20
";
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(@"
[#include]
+= schema.yini

[Config]
val = 5
");
            var validator = new Validator();

            var ex = Assert.Throws<ValidationException>(() => validator.Validate(doc));
            Assert.Contains("less than min 10", ex.Message);
        }

        [Fact]
        public void TestSerialization()
        {
            string source = @"[Config]
key = 1
val = ""text""
list = [1, 2, 3]

[#define]
MACRO = 100
";
            var compiler = new Compiler();
            var doc = compiler.Compile(source);

            var serializer = new Serializer();
            string output = serializer.Serialize(doc);

            Assert.Contains("[Config]", output);
            Assert.Contains("key = 1", output);
            Assert.Contains("val = \"text\"", output);
            Assert.Contains("list = [1, 2, 3]", output);
            Assert.Contains("[#define]", output);
            Assert.Contains("MACRO = 100", output);
        }

        [Fact]
        public void TestValidator_ArrayType()
        {
            var loader = new MockFileLoader();
            loader.Files["schema.yini"] = @"
[#schema]
[Config]
vals = !, array[int]
";
            var compiler = new Compiler(loader);
            var doc = compiler.Compile(@"
[#include]
+= schema.yini

[Config]
vals = [1, 2, 3]
");
            var validator = new Validator();
            validator.Validate(doc); // Should pass

            // Fail case
            var docFail = compiler.Compile(@"
[#include]
+= schema.yini

[Config]
vals = [1, ""bad"", 3]
");
             var ex = Assert.Throws<ValidationException>(() => validator.Validate(docFail));
             Assert.Contains("expected type int", ex.Message);
        }
    }
}
