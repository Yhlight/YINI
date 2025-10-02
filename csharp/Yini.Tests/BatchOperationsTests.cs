using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;

namespace Yini.Tests
{
    [TestClass]
    public class BatchOperationsTests
    {
        private const string TestFileName = "csharp_batch_test.yini";

        [TestCleanup]
        public void Cleanup()
        {
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [TestMethod]
        public void SetDoubles_UpdatesMultipleValues()
        {
            string originalContent = @"
[Settings]
volume = Dyna(100)
brightness = Dyna(0.8)
";
            File.WriteAllText(TestFileName, originalContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);
                manager.SetDoubles(
                    ("Settings", "volume", 50),
                    ("Settings", "brightness", 0.5)
                );
                manager.SaveChanges();
            }

            string newContent = File.ReadAllText(TestFileName);
            Assert.IsTrue(newContent.Contains("volume = 50"), "Batch double update for volume failed.");
            Assert.IsTrue(newContent.Contains("brightness = 0.5"), "Batch double update for brightness failed.");
        }

        [TestMethod]
        public void SetStrings_UpdatesMultipleValues()
        {
            string originalContent = @"
[Player]
name = Dyna(""Jules"")
class = Dyna(""Engineer"")
";
            File.WriteAllText(TestFileName, originalContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);
                manager.SetStrings(
                    ("Player", "name", "Jules Verne"),
                    ("Player", "class", "Author")
                );
                manager.SaveChanges();
            }

            string newContent = File.ReadAllText(TestFileName);
            Assert.IsTrue(newContent.Contains("name = \"Jules Verne\""), "Batch string update for name failed.");
            Assert.IsTrue(newContent.Contains("class = \"Author\""), "Batch string update for class failed.");
        }

        [TestMethod]
        public void SetBools_UpdatesMultipleValues()
        {
            string originalContent = @"
[Features]
enabled = Dyna(true)
visible = Dyna(false)
";
            File.WriteAllText(TestFileName, originalContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);
                manager.SetBools(
                    ("Features", "enabled", false),
                    ("Features", "visible", true)
                );
                manager.SaveChanges();
            }

            string newContent = File.ReadAllText(TestFileName);
            Assert.IsTrue(newContent.Contains("enabled = false"), "Batch bool update for enabled failed.");
            Assert.IsTrue(newContent.Contains("visible = true"), "Batch bool update for visible failed.");
        }
    }
}