using Xunit;
using Yini.Core;
using System.IO;
using System.Collections.Generic;

namespace Yini.Core.Tests
{
    public class ComplexTypeTests
    {
        private const string TestFileName = "complex_type_test.yini";

        private void CreateTestFile()
        {
            string content = @"
[Maps]
simpleMap = {key1: ""value1"", key2: 123, key3: true}
emptyMap = {}

[Structs]
simpleStruct = {key: ""value""}
";
            File.WriteAllText(TestFileName, content);
        }

        [Fact]
        public void TestGetMap()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                var map = config.GetMap("Maps.simpleMap");
                Assert.NotNull(map);
                Assert.Equal(3, map.Count);
                Assert.Equal("value1", map["key1"]);
                Assert.Equal(123, map["key2"]);
                Assert.Equal(true, map["key3"]);

                var emptyMap = config.GetMap("Maps.emptyMap");
                Assert.NotNull(emptyMap);
                Assert.Empty(emptyMap);
            }

            File.Delete(TestFileName);
        }

        [Fact]
        public void TestGetStruct()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                var aStruct = config.GetStruct("Structs.simpleStruct");
                Assert.NotNull(aStruct);
                Assert.Equal("key", aStruct.Value.Key);
                Assert.Equal("value", aStruct.Value.Value);
            }

            File.Delete(TestFileName);
        }
    }
}
