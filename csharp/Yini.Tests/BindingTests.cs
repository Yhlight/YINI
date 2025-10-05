using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;
using System.Numerics;

namespace Yini.Tests
{
    public class PlayerStats
    {
        public string Name { get; set; } = "";
        public int Level { get; set; }
        public double Health { get; set; }
        public bool IsActive { get; set; }
    }

    public class GameConfig
    {
        [YiniKey("window_title")]
        public string Title { get; set; } = "";

        [YiniKey("resolution_x")]
        public int Width { get; set; }

        [YiniKey("resolution_y")]
        public int Height { get; set; }

        public bool VSync { get; set; } // This will use the default "vsync" key
    }

    public class InventoryData
    {
        public List<string> Items { get; set; } = new();
        public Dictionary<string, int> Ammo { get; set; } = new();
    }

    public class AdvancedBindingData
    {
        public Color PlayerColor { get; set; }
        public Vector2 SpawnPoint { get; set; }
        public Vector3 SpawnRotation { get; set; }
        public Vector4 CameraFrustum { get; set; }
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

        [TestMethod]
        public void Bind_UsesYiniKeyAttribute()
        {
            string fileContent = @"
[graphics]
window_title = ""My Game""
resolution_x = 1920
resolution_y = 1080
vsync = true
";
            File.WriteAllText(TestFileName, fileContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                GameConfig config = manager.Bind<GameConfig>("graphics");

                Assert.IsNotNull(config);
                Assert.AreEqual("My Game", config.Title);
                Assert.AreEqual(1920, config.Width);
                Assert.AreEqual(1080, config.Height);
                Assert.IsTrue(config.VSync);
            }
        }

        [TestMethod]
        public void Bind_HandlesComplexTypes()
        {
            string fileContent = @"
[inventory]
items = [""sword"", ""shield"", ""potion""]
ammo = {""arrows"": 50, ""bolts"": 20}
";
            File.WriteAllText(TestFileName, fileContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                InventoryData inventory = manager.Bind<InventoryData>("inventory");

                Assert.IsNotNull(inventory);
                Assert.IsNotNull(inventory.Items);
                Assert.AreEqual(3, inventory.Items.Count);
                CollectionAssert.AreEqual(new List<string> { "sword", "shield", "potion" }, inventory.Items);

                Assert.IsNotNull(inventory.Ammo);
                Assert.AreEqual(2, inventory.Ammo.Count);
                Assert.AreEqual(50, inventory.Ammo["arrows"]);
                Assert.AreEqual(20, inventory.Ammo["bolts"]);
            }
        }

        [TestMethod]
        public void Bind_HandlesAdvancedStructTypes()
        {
            string fileContent = @"
[advanced]
playercolor = Color(255, 128, 64)
spawnpoint = Vec2(10.5, 20.2)
spawnrotation = Vec3(0, 90, 0)
camerafrustum = Vec4(0.1, 1000, 45, 1.77)
";
            File.WriteAllText(TestFileName, fileContent);

            using (var manager = new YiniManager())
            {
                manager.Load(TestFileName);

                AdvancedBindingData advanced = manager.Bind<AdvancedBindingData>("advanced");

                Assert.IsNotNull(advanced);
                Assert.AreEqual(new Color(255, 128, 64, 255), advanced.PlayerColor);
                Assert.AreEqual(new Vector2(10.5f, 20.2f), advanced.SpawnPoint);
                Assert.AreEqual(new Vector3(0, 90, 0), advanced.SpawnRotation);
                Assert.AreEqual(new Vector4(0.1f, 1000f, 45f, 1.77f), advanced.CameraFrustum);
            }
        }
    }
}