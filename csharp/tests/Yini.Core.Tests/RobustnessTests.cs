using Xunit;
using Yini.Core;
using System.IO;

namespace Yini.Core.Tests
{
    public class RobustnessTests
    {
        private const string TestFileName = "robustness_test.yini";

        [Fact]
        public void ThrowsExceptionForNonExistentFile()
        {
            var exception = Assert.Throws<YiniException>(() => new YiniConfig("non_existent_file.yini"));
            Assert.Contains("Failed to create YINI config", exception.Message);
        }

        [Fact]
        public void ThrowsExceptionForInvalidSyntax()
        {
            File.WriteAllText(TestFileName, "[Section"); // Missing closing bracket
            var exception = Assert.Throws<YiniException>(() => new YiniConfig(TestFileName));
            Assert.Contains("Error at line 1", exception.Message);
            File.Delete(TestFileName);
        }

        [Fact]
        public void CanParseColor()
        {
            File.WriteAllText(TestFileName, "[Test]\nvalue = #FFC0CB");
            using var config = new YiniConfig(TestFileName);
            Assert.NotNull(config);
            File.Delete(TestFileName);
        }

        [Fact]
        public void CanParseCoord()
        {
            File.WriteAllText(TestFileName, "[Test]\nvalue = coord(1, 2)");
            using var config = new YiniConfig(TestFileName);
            Assert.NotNull(config);
            File.Delete(TestFileName);
        }

        [Fact]
        public void CanParseMap()
        {
            File.WriteAllText(TestFileName, "[Test]\nvalue = { k: \"v\" }");
            using var config = new YiniConfig(TestFileName);
            Assert.NotNull(config);
            File.Delete(TestFileName);
        }

        [Fact]
        public void CanParseNestedArray()
        {
            File.WriteAllText(TestFileName, "[Test]\nvalue = [[1, 2], [3, 4]]");
            using var config = new YiniConfig(TestFileName);
            Assert.NotNull(config);
            File.Delete(TestFileName);
        }
    }
}
