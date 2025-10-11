using Xunit;
using Yini.Core;
using System.IO;

namespace Yini.Core.Tests
{
    public class YiniTests
    {
        private const string TestFileName = "csharp_test.yini";

        private void CreateTestFile()
        {
            string content = @"
[TestSection]
intValue = 123
doubleValue = 45.67
boolValue = true
stringValue = ""hello world""
";
            File.WriteAllText(TestFileName, content);
        }

        [Fact]
        public void TestGetValues()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                // Test for existing keys
                Assert.Equal(123, config.GetInt("TestSection.intValue"));
                Assert.Equal(45.67, config.GetDouble("TestSection.doubleValue"));
                Assert.True(config.GetBool("TestSection.boolValue"));
                Assert.Equal("hello world", config.GetString("TestSection.stringValue"));

                // Test for non-existent keys
                Assert.Null(config.GetInt("TestSection.nonExistentInt"));
                Assert.Null(config.GetDouble("TestSection.nonExistentDouble"));
                Assert.Null(config.GetBool("TestSection.nonExistentBool"));
                Assert.Null(config.GetString("TestSection.nonExistentString"));
            }

            File.Delete(TestFileName);
        }
    }
}
