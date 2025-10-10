using Xunit;
using Yini.Core;
using System.IO;

namespace Yini.Core.Tests
{
    public class PrimitiveArrayTests
    {
        private const string TestFileName = "primitive_array_test.yini";

        private void CreateTestFile()
        {
            string content = @"
[Arrays]
intArray = [1, 2, 3]
doubleArray = [1.1, 2.2, 3.3]
boolArray = [true, false, true]
";
            File.WriteAllText(TestFileName, content);
        }

        [Fact]
        public void TestGetPrimitiveArrays()
        {
            CreateTestFile();

            using (var config = new YiniConfig(TestFileName))
            {
                // Int array test
                int[] intArray = config.GetIntArray("Arrays.intArray");
                Assert.NotNull(intArray);
                Assert.Equal(3, intArray.Length);
                Assert.Equal(1, intArray[0]);
                Assert.Equal(2, intArray[1]);
                Assert.Equal(3, intArray[2]);

                // Double array test
                double[] doubleArray = config.GetDoubleArray("Arrays.doubleArray");
                Assert.NotNull(doubleArray);
                Assert.Equal(3, doubleArray.Length);
                Assert.Equal(1.1, doubleArray[0]);
                Assert.Equal(2.2, doubleArray[1]);
                Assert.Equal(3.3, doubleArray[2]);

                // Bool array test
                bool[] boolArray = config.GetBoolArray("Arrays.boolArray");
                Assert.NotNull(boolArray);
                Assert.Equal(3, boolArray.Length);
                Assert.True(boolArray[0]);
                Assert.False(boolArray[1]);
                Assert.True(boolArray[2]);
            }

            File.Delete(TestFileName);
        }
    }
}