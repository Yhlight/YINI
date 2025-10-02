using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;

namespace Yini.Tests
{
    public class PlayerStats
    {
        public string Name { get; set; }
        public int Level { get; set; }
        public double Health { get; set; }
        public bool IsActive { get; set; }
    }

    [TestClass]
    public class BindingTests
    {
        private const string TestFileName = "csharp_binding_test.yini";

        [TestCleanup]
        public void Cleanup()
        {
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [TestMethod]
        public void Bind_PopulatesObjectCorrectly()
        {
            string fileContent = @"
[playerstats]
name = Jules
level = 99
health = 125.5
isactive = true
";
            File.WriteAllText(TestFileName, fileContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                PlayerStats stats = manager.Bind<PlayerStats>("playerstats");

                Assert.IsNotNull(stats);
                Assert.AreEqual("Jules", stats.Name);
                Assert.AreEqual(99, stats.Level);
                Assert.AreEqual(125.5, stats.Health);
                Assert.IsTrue(stats.IsActive);
            }
        }
    }
}