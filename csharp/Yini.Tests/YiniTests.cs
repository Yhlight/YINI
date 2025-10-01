using NUnit.Framework;
using YINI;
using System;
using System.IO;
using System.Collections.Generic;

namespace Yini.Tests
{
    public class YiniTests
    {
        private const string TestContent = @"
[#define]
version = 1.2
factor = 2

[Core]
name = ""YINI Engine""
enabled = true
version = @version
factor = @factor
data = [1, ""two"", true, 4.5]
pos = Coord(10, 20)
color = #00FF00
asset = Path(items/sword.mesh)
scores = List(100, 95, 80)
tags = Set(""fast"", ""player"", ""fast"")
metadata = { author: ""Jules"", year: 2024 }

[Values]
rate = 12.5 * @factor
";

        [Test]
        public void ParseDocument_ShouldReturnCorrectSectionCount()
        {
            using (var doc = new YiniDocument(TestContent))
            {
                Assert.That(doc.SectionCount, Is.EqualTo(2));
            }
        }

        [Test]
        public void GetDefines_ShouldReturnCorrectMacros()
        {
            using (var doc = new YiniDocument(TestContent))
            {
                var defines = doc.GetDefines();
                Assert.That(defines.Count, Is.EqualTo(2));
                Assert.That(defines.ContainsKey("version"), Is.True);
                Assert.That(defines["version"].AsDouble(), Is.EqualTo(1.2));
                Assert.That(defines.ContainsKey("factor"), Is.True);
                Assert.That(defines["factor"].AsInt(), Is.EqualTo(2));
            }
        }

        [Test]
        public void GetSectionByName_ShouldReturnCorrectSection()
        {
            using (var doc = new YiniDocument(TestContent))
            {
                var coreSection = doc.GetSection("Core");
                Assert.That(coreSection, Is.Not.Null);
                Assert.That(coreSection.Name, Is.EqualTo("Core"));

                var nonExistent = doc.GetSection("NonExistent");
                Assert.That(nonExistent, Is.Null);
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectValueAndType()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var nameValue = doc.GetValue("Core", "name");
                Assert.That(nameValue, Is.Not.Null);
                Assert.That(nameValue!.Type, Is.EqualTo(YiniType.String));
                Assert.That(nameValue.AsString(), Is.EqualTo("YINI Engine"));

                var enabledValue = doc.GetValue("Core", "enabled");
                Assert.That(enabledValue, Is.Not.Null);
                Assert.That(enabledValue!.Type, Is.EqualTo(YiniType.Bool));
                Assert.That(enabledValue.AsBool(), Is.True);

                var factorValue = doc.GetValue("Core", "factor");
                Assert.That(factorValue, Is.Not.Null);
                Assert.That(factorValue!.Type, Is.EqualTo(YiniType.Int));
                Assert.That(factorValue.AsInt(), Is.EqualTo(2));

                var versionValue = doc.GetValue("Core", "version");
                Assert.That(versionValue, Is.Not.Null);
                Assert.That(versionValue!.Type, Is.EqualTo(YiniType.Double));
                Assert.That(versionValue.AsDouble(), Is.EqualTo(1.2));

                var rateValue = doc.GetValue("Values", "rate");
                Assert.That(rateValue, Is.Not.Null);
                Assert.That(rateValue!.Type, Is.EqualTo(YiniType.Double));
                Assert.That(rateValue.AsDouble(), Is.EqualTo(25.0));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectArray()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var dataValue = doc.GetValue("Core", "data");
                Assert.That(dataValue, Is.Not.Null);
                Assert.That(dataValue!.Type, Is.EqualTo(YiniType.Array));

                var array = dataValue.AsArray();
                Assert.That(array.Length, Is.EqualTo(4));
                Assert.That(array[0].AsInt(), Is.EqualTo(1));
                Assert.That(array[1].AsString(), Is.EqualTo("two"));
                Assert.That(array[2].AsBool(), Is.True);
                Assert.That(array[3].AsDouble(), Is.EqualTo(4.5));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectList()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var dataValue = doc.GetValue("Core", "scores");
                Assert.That(dataValue, Is.Not.Null);
                Assert.That(dataValue!.Type, Is.EqualTo(YiniType.List));

                var list = dataValue.AsList();
                Assert.That(list.Length, Is.EqualTo(3));
                Assert.That(list[0].AsInt(), Is.EqualTo(100));
                Assert.That(list[1].AsInt(), Is.EqualTo(95));
                Assert.That(list[2].AsInt(), Is.EqualTo(80));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectSet()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var dataValue = doc.GetValue("Core", "tags");
                Assert.That(dataValue, Is.Not.Null);
                Assert.That(dataValue!.Type, Is.EqualTo(YiniType.Set));

                var set = dataValue.AsSet();
                Assert.That(set.Length, Is.EqualTo(2));
                Assert.That(set[0].AsString(), Is.EqualTo("fast"));
                Assert.That(set[1].AsString(), Is.EqualTo("player"));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectMap()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var dataValue = doc.GetValue("Core", "metadata");
                Assert.That(dataValue, Is.Not.Null);
                Assert.That(dataValue!.Type, Is.EqualTo(YiniType.Map));

                var map = dataValue.AsMap();
                Assert.That(map.Count, Is.EqualTo(2));
                Assert.That(map.ContainsKey("author"), Is.True);
                Assert.That(map["author"].AsString(), Is.EqualTo("Jules"));
                Assert.That(map.ContainsKey("year"), Is.True);
                Assert.That(map["year"].AsInt(), Is.EqualTo(2024));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectCustomTypes()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var posValue = doc.GetValue("Core", "pos");
                Assert.That(posValue, Is.Not.Null);
                Assert.That(posValue!.Type, Is.EqualTo(YiniType.Coord));
                var coord = posValue.AsCoord();
                Assert.That(coord.X, Is.EqualTo(10));
                Assert.That(coord.Y, Is.EqualTo(20));
                Assert.That(coord.Is3D, Is.False);

                var colorValue = doc.GetValue("Core", "color");
                Assert.That(colorValue, Is.Not.Null);
                Assert.That(colorValue!.Type, Is.EqualTo(YiniType.Color));
                var color = colorValue.AsColor();
                Assert.That(color.R, Is.EqualTo(0));
                Assert.That(color.G, Is.EqualTo(255));
                Assert.That(color.B, Is.EqualTo(0));

                var assetValue = doc.GetValue("Core", "asset");
                Assert.That(assetValue, Is.Not.Null);
                Assert.That(assetValue!.Type, Is.EqualTo(YiniType.Path));
                Assert.That(assetValue.AsPath(), Is.EqualTo("items/sword.mesh"));
            }
        }

        [Test]
        public void GetValue_InvalidCast_ShouldThrow()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var nameValue = doc.GetValue("Core", "name");
                Assert.That(nameValue, Is.Not.Null);
                Assert.Throws<InvalidCastException>(() => nameValue!.AsInt());
                Assert.Throws<InvalidCastException>(() => nameValue!.AsArray());
                Assert.Throws<InvalidCastException>(() => nameValue!.AsCoord());
            }
        }

        [Test]
        public void SetValue_ShouldModifyAndAddValues()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                // Modify existing value
                doc.SetValue("Core", "name", "New YINI");
                var modifiedName = doc.GetValue("Core", "name");
                Assert.That(modifiedName, Is.Not.Null);
                Assert.That(modifiedName.Type, Is.EqualTo(YiniType.String));
                Assert.That(modifiedName.AsString(), Is.EqualTo("New YINI"));

                // Add new value to existing section
                doc.SetValue("Core", "new_key", 999);
                var newValue = doc.GetValue("Core", "new_key");
                Assert.That(newValue, Is.Not.Null);
                Assert.That(newValue.Type, Is.EqualTo(YiniType.Int));
                Assert.That(newValue.AsInt(), Is.EqualTo(999));

                // Add new value to new section
                doc.SetValue("NewSection", "data", true);
                var newSectionValue = doc.GetValue("NewSection", "data");
                Assert.That(newSectionValue, Is.Not.Null);
                Assert.That(newSectionValue.Type, Is.EqualTo(YiniType.Bool));
                Assert.That(newSectionValue.AsBool(), Is.True);
                Assert.That(doc.SectionCount, Is.EqualTo(3));
            }
        }

        [Test]
        public void YiniManager_LoadsAndModifiesDynaValue_PreservesFormatting()
        {
            const string yiniPath = "manager_csharp_test.yini";
            const string initialContent = "[Settings]\n# Test comment\n  volume = Dyna(100)\n";
            const string expectedContent = "[Settings]\n# Test comment\n  volume = Dyna(75)";

            try
            {
                File.WriteAllText(yiniPath, initialContent);

                // Scope the manager so its destructor is called
                using (var manager = new YiniManager(yiniPath))
                {
                    Assert.That(manager.IsLoaded, Is.True);
                    manager.SetValue("Settings", "volume", 75);
                }

                string finalContent = File.ReadAllText(yiniPath);
                Assert.That(finalContent, Is.EqualTo(expectedContent));
            }
            finally
            {
                if (File.Exists(yiniPath))
                {
                    File.Delete(yiniPath);
                }
                // Clean up ymeta file as well
                if (File.Exists("manager_csharp_test.ymeta"))
                {
                    File.Delete("manager_csharp_test.ymeta");
                }
                 if (File.Exists("manager_csharp_test.ymeta.bak1"))
                {
                    File.Delete("manager_csharp_test.ymeta.bak1");
                }
            }
        }
    }
}