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
                Assert.True(config.GetInt("TestSection.intValue", out int intValue));
                Assert.Equal(123, intValue);

                Assert.True(config.GetDouble("TestSection.doubleValue", out double doubleValue));
                Assert.Equal(45.67, doubleValue);

                Assert.True(config.GetBool("TestSection.boolValue", out bool boolValue));
                Assert.True(boolValue);

                string stringValue = config.GetString("TestSection.stringValue");
                Assert.Equal("hello world", stringValue);
            }

            File.Delete(TestFileName);
        }
    }
}