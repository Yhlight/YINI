using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;
using System.Collections.Generic;
using System.Numerics;

namespace Yini.Tests
{
    [TestClass]
    public class YiniManagerTests
    {
        private const string TestFileName = "csharp_test.yini";

        [TestCleanup]
        public void Cleanup()
        {
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [TestMethod]
        public void FullLifecycle_LoadGetSetAndSaveChanges()
        {
            // 1. Create a test .yini file
            string originalContent = @"
// C# Test File
[Settings]
volume = Dyna(100) // Dynamic master volume
rate = 1.0
";
            File.WriteAllText(TestFileName, originalContent);

            // 2. Load the file using the manager
            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName); // This will throw on failure

                // 3. Get values
                Assert.AreEqual(100, manager.GetDouble("Settings", "volume"));
                Assert.AreEqual(1.0, manager.GetDouble("Settings", "rate"));

                // 4. Set a new value for the dynamic key
                manager.SetDouble("Settings", "volume", 50);
                Assert.AreEqual(50, manager.GetDouble("Settings", "volume"));

                // 5. Save the changes
                manager.SaveChanges();
            }

            // 6. Read the file back and verify its contents
            string newContent = File.ReadAllText(TestFileName);

            // Check that the dynamic value was updated
            Assert.IsTrue(newContent.Contains("volume = 50 "), "The dynamic value was not updated correctly.");

            // Check that comments and other lines are preserved
            Assert.IsTrue(newContent.Contains("// C# Test File"), "The top-level comment was not preserved.");
            Assert.IsTrue(newContent.Contains("// Dynamic master volume"), "The inline comment was not preserved.");
            Assert.IsTrue(newContent.Contains("rate = 1.0"), "The non-dynamic value was not preserved.");
        }

        [TestMethod]
        public void GetString_ReturnsCorrectValue()
        {
            // 1. Create a test .yini file with a string value
            string longString = new string('a', 2048);
            string fileContent = $@"
[Strings]
short = ""Hello""
long = ""{longString}""
";
            File.WriteAllText(TestFileName, fileContent);

            // 2. Load the file
            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                // 3. Get the string values
                string shortResult = manager.GetString("Strings", "short");
                string longResult = manager.GetString("Strings", "long");
                string nonExistentResult = manager.GetString("Strings", "nonexistent", "default");

                // 4. Assert correctness
                Assert.AreEqual("Hello", shortResult);
                Assert.AreEqual(longString, longResult);
                Assert.AreEqual("default", nonExistentResult);
            }
        }

        [TestMethod]
        public void Get_Generic_ReturnsCorrectTypes()
        {
            // 1. Create a test .yini file with various types
            string fileContent = @"
[AllTypes]
string_val = ""Hello Generic""
int_val = 123
double_val = 45.6
bool_val = true
list_val = [""one"", ""two"", ""three""]
map_val = { ""a"": 1, ""b"": 2 }
vector_val = Vec3(1.1, 2.2, 3.3)
color_val = Color(255, 128, 64)
";
            File.WriteAllText(TestFileName, fileContent);

            // 2. Load the file
            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                // 3. Get values using the generic Get<T> method
                var stringResult = manager.Get<string>("AllTypes", "string_val");
                var intResult = manager.Get<int>("AllTypes", "int_val");
                var doubleResult = manager.Get<double>("AllTypes", "double_val");
                var boolResult = manager.Get<bool>("AllTypes", "bool_val");
                var listResult = manager.Get<List<string>>("AllTypes", "list_val");
                var mapResult = manager.Get<Dictionary<string, int>>("AllTypes", "map_val");
                var vectorResult = manager.Get<Vector3>("AllTypes", "vector_val");
                var colorResult = manager.Get<Color>("AllTypes", "color_val");
                var defaultResult = manager.Get<string>("AllTypes", "nonexistent", "default_val");

                // 4. Assert correctness
                Assert.AreEqual("Hello Generic", stringResult);
                Assert.AreEqual(123, intResult);
                Assert.AreEqual(45.6, doubleResult);
                Assert.IsTrue(boolResult);

                Assert.IsNotNull(listResult);
                CollectionAssert.AreEqual(new List<string> { "one", "two", "three" }, listResult);

                Assert.IsNotNull(mapResult);
                Assert.AreEqual(2, mapResult.Count);
                Assert.AreEqual(1, mapResult["a"]);
                Assert.AreEqual(2, mapResult["b"]);

                Assert.AreEqual(new Vector3(1.1f, 2.2f, 3.3f), vectorResult);
                Assert.AreEqual(new Color(255, 128, 64, 255), colorResult);

                Assert.AreEqual("default_val", defaultResult);
            }
        }

        [TestMethod]
        public void Load_NonExistentFile_ThrowsYiniException()
        {
            // Arrange
            using var manager = new YiniManager();
            var filePath = "non_existent_file.yini";

            // Act & Assert
            var ex = Assert.ThrowsException<YiniException>(() => manager.Load(filePath));
            StringAssert.Contains(ex.Message, "Could not open file");
        }
    }
}
