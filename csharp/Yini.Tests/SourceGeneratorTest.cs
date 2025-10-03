using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;

namespace Yini.Tests
{
    [YiniBindable]
    public partial class TestBindable
    {
        public string Name { get; set; } = "Default";
        public int Level { get; set; } = 0;
        public bool IsActive { get; set; } = false;

        [YiniKey("character_class")]
        public string Class { get; set; } = "Peasant";
    }

    [TestClass]
    public class SourceGeneratorTest
    {
        private const string TestFileName = "sg_test.yini";

        [TestInitialize]
        public void Setup()
        {
            var content = @"
[player]
name = Jules
level = 99
isactive = true
character_class = Warrior
";
            File.WriteAllText(TestFileName, content);
        }

        [TestCleanup]
        public void Cleanup()
        {
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [TestMethod]
        public void GeneratedBinder_BindsDataCorrectly()
        {
            // Arrange
            var manager = new YiniManager();
            manager.Load(TestFileName);
            var player = new TestBindable();

            // Act
            player.BindFromYini(manager, "player");

            // Assert
            Assert.AreEqual("Jules", player.Name);
            Assert.AreEqual(99, player.Level);
            Assert.AreEqual(true, player.IsActive);
            Assert.AreEqual("Warrior", player.Class);
        }
    }
}