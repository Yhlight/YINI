using Xunit;
using Yini.Core;
using System.IO;

namespace Yini.Core.Tests
{
    public class ListTypeTests
    {
        private const string TestFileName = "list_type_test.yini";

        private void CreateTestFile()
        {
            string content = @"
[Lists]
intList = list(1, 2, 3)
doubleList = list(1.1, 2.2, 3.3)
boolList = list(true, false, true)
stringList = list(""one"", ""two"", ""three"")
";
            File.WriteAllText(TestFileName, content);
        }

        [Fact]
        public void TestGetList()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                // Int list test
                int?[]? intList = config.GetIntList("Lists.intList");
                Assert.NotNull(intList);
                Assert.Equal(3, intList.Length);
                Assert.Equal(1, intList[0]);
                Assert.Equal(2, intList[1]);
                Assert.Equal(3, intList[2]);

                // Double list test
                double?[]? doubleList = config.GetDoubleList("Lists.doubleList");
                Assert.NotNull(doubleList);
                Assert.Equal(3, doubleList.Length);
                Assert.Equal(1.1, doubleList[0]);
                Assert.Equal(2.2, doubleList[1]);
                Assert.Equal(3.3, doubleList[2]);

                // Bool list test
                bool?[]? boolList = config.GetBoolList("Lists.boolList");
                Assert.NotNull(boolList);
                Assert.Equal(3, boolList.Length);
                Assert.True(boolList[0]);
                Assert.False(boolList[1]);
                Assert.True(boolList[2]);

                // String list test
                string?[]? stringList = config.GetStringList("Lists.stringList");
                Assert.NotNull(stringList);
                Assert.Equal(3, stringList.Length);
                Assert.Equal("one", stringList[0]);
                Assert.Equal("two", stringList[1]);
                Assert.Equal("three", stringList[2]);
            }

            File.Delete(TestFileName);
        }
    }
}
