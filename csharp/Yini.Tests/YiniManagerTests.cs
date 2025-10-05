using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;

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
