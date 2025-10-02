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
                bool loaded = manager.Load(TestFileName);
                Assert.IsTrue(loaded, "Failed to load the .yini file.");

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
            Assert.IsTrue(newContent.Contains("volume = 50.000000"), "The dynamic value was not updated correctly.");

            // Check that comments and other lines are preserved
            Assert.IsTrue(newContent.Contains("// C# Test File"), "The top-level comment was not preserved.");
            Assert.IsTrue(newContent.Contains("// Dynamic master volume"), "The inline comment was not preserved.");
            Assert.IsTrue(newContent.Contains("rate = 1.0"), "The non-dynamic value was not preserved.");
        }
    }
}