using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;

namespace Yini.Tests
{
    [YiniBindable]
    public partial class SourceGenPlayer
    {
        // This will use the default snake_case conversion to "player_name"
        public string PlayerName { get; set; } = "Default";

        // This will also use the default conversion to "is_active"
        public bool IsActive { get; set; } = false;

        // This will use the explicit key from the attribute, overriding the default "player_class"
        [YiniKey("character_class")]
        public string PlayerClass { get; set; } = "Peasant";
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
// This key is snake_case and should map to PlayerName
player_name = ""Jules""

// This key is also snake_case and should map to IsActive
is_active = true

// This key is explicitly defined by the YiniKey attribute
character_class = ""Warrior""
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
        public void GeneratedBinder_BindsWithDefaultSnakeCaseAndAttributeOverride()
        {
            // Arrange
            var manager = new YiniManager();
            manager.Load(TestFileName);
            var player = new SourceGenPlayer();

            // Act
            player.BindFromYini(manager, "player");

            // Assert
            Assert.AreEqual("Jules", player.PlayerName);
            Assert.AreEqual(true, player.IsActive);
            Assert.AreEqual("Warrior", player.PlayerClass);
        }
    }
}
