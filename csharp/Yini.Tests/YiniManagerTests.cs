using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using System.Collections.Generic;
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

                // 4. Set a new value for the dynamic key and a comment
                manager.SetDouble("Settings", "volume", 50);
                Assert.AreEqual(50, manager.GetDouble("Settings", "volume"));

                // 5. Save the changes
                manager.SaveChanges();
            }

            // 6. Re-load the file with a new manager and verify the content semantically
            using (var verifyManager = new YiniManager())
            {
                verifyManager.Load(TestFileName);

                // Check that the dynamic value was updated
                Assert.AreEqual(50, verifyManager.GetDouble("Settings", "volume"), "The dynamic value was not updated correctly.");

                // Check that the non-dynamic value is still correct
                Assert.AreEqual(1.0, verifyManager.GetDouble("Settings", "rate"), "The non-dynamic value was not preserved.");

                // Check that comments were preserved
                Assert.AreEqual(" C# Test File", verifyManager.GetSectionDocComment("Settings"), "The doc comment was not preserved.");
                Assert.AreEqual(" Dynamic master volume", verifyManager.GetKeyInlineComment("Settings", "volume"), "The inline comment was not preserved.");
            }
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
        public void CommentAccess_GetsAndSetsComments()
        {
            string fileContent = @"
// Section Comment
[Comments]
// Key Comment
key = 1 // Inline Comment
";
            File.WriteAllText(TestFileName, fileContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                // Get comments
                Assert.AreEqual(" Section Comment", manager.GetSectionDocComment("Comments"));
                Assert.AreEqual(" Key Comment", manager.GetKeyDocComment("Comments", "key"));
                Assert.AreEqual(" Inline Comment", manager.GetKeyInlineComment("Comments", "key"));

                // Set comments
                manager.SetSectionDocComment("Comments", "New Section Comment");
                manager.SetKeyDocComment("Comments", "key", "New Key Comment");
                manager.SetKeyInlineComment("Comments", "key", "New Inline Comment");

                manager.SaveChanges();
            }

            // Verify changes were saved
            using (var verifyManager = new YiniManager())
            {
                verifyManager.Load(TestFileName);
                Assert.AreEqual("New Section Comment", verifyManager.GetSectionDocComment("Comments"));
                Assert.AreEqual("New Key Comment", verifyManager.GetKeyDocComment("Comments", "key"));
                Assert.AreEqual("New Inline Comment", verifyManager.GetKeyInlineComment("Comments", "key"));
            }
        }
    }
}