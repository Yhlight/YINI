using Xunit;
using Yini.Core;
using System.IO;

namespace Yini.Core.Tests
{
    public class ComplexTypeTests
    {
        private const string TestFileName = "complex_types_test.yini";

        private void CreateTestFile()
        {
            string content = @"
[ComplexTypes]
colorValue = #FF8000
coordValue = coord(1.2, 3.4, 5.6)
stringArray = [""one"", ""two"", ""three""]
";
            File.WriteAllText(TestFileName, content);
        }

        [Fact]
        public void TestGetComplexTypes()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                // Color test
                Assert.True(config.GetColor("ComplexTypes.colorValue", out YiniColor colorValue));
                Assert.Equal(255, colorValue.r);
                Assert.Equal(128, colorValue.g);
                Assert.Equal(0, colorValue.b);

                // Coord test
                Assert.True(config.GetCoord("ComplexTypes.coordValue", out YiniCoord coordValue));
                Assert.Equal(1.2, coordValue.x);
                Assert.Equal(3.4, coordValue.y);
                Assert.True(coordValue.has_z);
                Assert.Equal(5.6, coordValue.z);

                // String array test
                string[] stringArray = config.GetStringArray("ComplexTypes.stringArray");
                Assert.NotNull(stringArray);
                Assert.Equal(3, stringArray.Length);
                Assert.Equal("one", stringArray[0]);
                Assert.Equal("two", stringArray[1]);
                Assert.Equal("three", stringArray[2]);
            }

            File.Delete(TestFileName);
        }
    }
}