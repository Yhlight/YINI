using NUnit.Framework;
using System.IO;
using Yini.Core;

namespace Yini.Core.Tests
{
    public class YiniConfigTests
    {
        private const string TestFileName = "test_config.yini";

        [SetUp]
        public void Setup()
        {
            File.WriteAllText(TestFileName, @"
[TestSection]
string_val = ""hello world""
int_val = 42
double_val = 3.14
bool_val = true

[Data]
array_val = [1, ""two"", 3.0, false]
map_val = { key1: ""value1"", key2: 100 }

[Dynamic]
level = Dyna(10)

[SpecialTypes]
player_color = color(255, 100, 50)
spawn_point = Coord(10, -20, 5)
texture_path = path(""textures/player.png"")
");
        }

        [TearDown]
        public void TearDown()
        {
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [Test]
        public void GetString_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Assert.AreEqual("hello world", config.GetString("TestSection", "string_val"));
            }
        }

        [Test]
        public void GetInt_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(42, config.GetInt("TestSection", "int_val"));
            }
        }

        [Test]
        public void GetDouble_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(3.14, config.GetDouble("TestSection", "double_val"));
            }
        }

        [Test]
        public void GetBool_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(true, config.GetBool("TestSection", "bool_val"));
            }
        }

        [Test]
        public void GetNonExistentValue_ReturnsDefault()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Assert.AreEqual("default", config.GetString("TestSection", "non_existent", "default"));
                Assert.AreEqual(100, config.GetInt("TestSection", "non_existent", 100));
                Assert.AreEqual(1.23, config.GetDouble("TestSection", "non_existent", 1.23));
                Assert.AreEqual(true, config.GetBool("TestSection", "non_existent", true));
            }
        }

        [Test]
        public void GetValue_ReturnsArrayCorrectly()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                var result = config.GetValue("Data", "array_val") as object[];
                Assert.IsNotNull(result);
                Assert.AreEqual(4, result.Length);
                Assert.AreEqual(1, result[0]);
                Assert.AreEqual("two", result[1]);
                Assert.AreEqual(3.0, result[2]);
                Assert.AreEqual(false, result[3]);
            }
        }

        [Test]
        public void GetValue_ReturnsMapCorrectly()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                var result = config.GetValue("Data", "map_val") as Dictionary<string, object>;
                Assert.IsNotNull(result);
                Assert.AreEqual(2, result.Count);
                Assert.AreEqual("value1", result["key1"]);
                Assert.AreEqual(100, result["key2"]);
            }
        }

        [Test]
        public void SetValue_And_Save_PersistsChanges()
        {
            // 1. Arrange & Act
            using (var config = new YiniConfig(TestFileName))
            {
                config.SetInt("TestSection", "int_val", 999);
                config.Save();
            }

            // 2. Assert
            using (var newConfig = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(999, newConfig.GetInt("TestSection", "int_val"));
            }
        }

        [Test]
        public void GetDyna_And_Update_WorksCorrectly()
        {
            // 1. Arrange & Act
            using (var config = new YiniConfig(TestFileName))
            {
                DynaValue<int> dynaLevel = config.GetDyna<int>("Dynamic", "level");

                // 2. Assert Initial State
                Assert.IsNotNull(dynaLevel);
                Assert.AreEqual(10, dynaLevel.Value);
                Assert.AreEqual(0, dynaLevel.Backups.Count);

                // 3. Act: Update value
                dynaLevel.Value = 11;

                // 4. Assert Updated State
                Assert.AreEqual(11, dynaLevel.Value);
                Assert.AreEqual(1, dynaLevel.Backups.Count);
                Assert.AreEqual(10, dynaLevel.Backups[0]);

                // 5. Act: Save and reload
                config.Save();
            }

            // 6. Assert Persistence
            using (var newConfig = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(11, newConfig.GetInt("Dynamic", "level"));

                // Also check if the backups were persisted in the DynaValue object
                DynaValue<int> reloadedDynaLevel = newConfig.GetDyna<int>("Dynamic", "level");
                Assert.IsNotNull(reloadedDynaLevel);
                Assert.AreEqual(11, reloadedDynaLevel.Value);
                Assert.AreEqual(1, reloadedDynaLevel.Backups.Count);
                Assert.AreEqual(10, reloadedDynaLevel.Backups[0]);
            }
        }

        [Test]
        public void DynaValue_AutoSavesOnDispose()
        {
            // Act: Modify the value within a using scope, which will trigger Dispose
            using (var config = new YiniConfig(TestFileName))
            {
                DynaValue<int> dynaLevel = config.GetDyna<int>("Dynamic", "level");
                dynaLevel.Value = 25;
            }

            // Assert: Verify the change was persisted after Dispose was called
            using (var newConfig = new YiniConfig(TestFileName))
            {
                Assert.AreEqual(25, newConfig.GetInt("Dynamic", "level"));
            }
        }

        [Test]
        public void GetColor_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Color color = config.GetColor("SpecialTypes", "player_color");
                Assert.AreEqual(new Color { r = 255, g = 100, b = 50 }, color);
            }
        }

        [Test]
        public void GetCoord_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                Coord coord = config.GetCoord("SpecialTypes", "spawn_point");
                Assert.AreEqual(new Coord { x = 10, y = -20, z = 5 }, coord);
            }
        }

        [Test]
        public void GetPath_ReturnsCorrectValue()
        {
            using (var config = new YiniConfig(TestFileName))
            {
                string path = config.GetPath("SpecialTypes", "texture_path");
                Assert.AreEqual("textures/player.png", path);
            }
        }

        [Test]
        public void SetColor_And_Save_PersistsChanges()
        {
            var newColor = new Color { r = 10, g = 20, b = 30 };
            using (var config = new YiniConfig(TestFileName))
            {
                config.SetColor("SpecialTypes", "player_color", newColor);
                config.Save();
            }

            using (var newConfig = new YiniConfig(TestFileName))
            {
                Color result = newConfig.GetColor("SpecialTypes", "player_color");
                Assert.AreEqual(newColor, result);
            }
        }

        [Test]
        public void SetCoord_And_Save_PersistsChanges()
        {
            var newCoord = new Coord { x = 1, y = 2, z = 3 };
            using (var config = new YiniConfig(TestFileName))
            {
                config.SetCoord("SpecialTypes", "spawn_point", newCoord);
                config.Save();
            }

            using (var newConfig = new YiniConfig(TestFileName))
            {
                Coord result = newConfig.GetCoord("SpecialTypes", "spawn_point");
                Assert.AreEqual(newCoord, result);
            }
        }

        [Test]
        public void SetPath_And_Save_PersistsChanges()
        {
            var newPath = "new/path/to/texture.png";
            using (var config = new YiniConfig(TestFileName))
            {
                config.SetPath("SpecialTypes", "texture_path", newPath);
                config.Save();
            }

            using (var newConfig = new YiniConfig(TestFileName))
            {
                string result = newConfig.GetPath("SpecialTypes", "texture_path");
                Assert.AreEqual(newPath, result);
            }
        }
    }
}