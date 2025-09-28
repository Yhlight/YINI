using NUnit.Framework;
using YINI;
using System;

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
                Assert.That(nameValue.Type, Is.EqualTo(YiniType.String));
                Assert.That(nameValue.AsString(), Is.EqualTo("YINI Engine"));

                var enabledValue = doc.GetValue("Core", "enabled");
                Assert.That(enabledValue.Type, Is.EqualTo(YiniType.Bool));
                Assert.That(enabledValue.AsBool(), Is.True);

                var factorValue = doc.GetValue("Core", "factor");
                Assert.That(factorValue.Type, Is.EqualTo(YiniType.Int));
                Assert.That(factorValue.AsInt(), Is.EqualTo(2));

                var versionValue = doc.GetValue("Core", "version");
                Assert.That(versionValue.Type, Is.EqualTo(YiniType.Double));
                Assert.That(versionValue.AsDouble(), Is.EqualTo(1.2));

                var rateValue = doc.GetValue("Values", "rate");
                Assert.That(rateValue.Type, Is.EqualTo(YiniType.Double));
                Assert.That(rateValue.AsDouble(), Is.EqualTo(25.0));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectArray()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var dataValue = doc.GetValue("Core", "data");
                Assert.That(dataValue.Type, Is.EqualTo(YiniType.Array));

                var array = dataValue.AsArray();
                Assert.That(array.Length, Is.EqualTo(4));
                Assert.That(array[0].AsInt(), Is.EqualTo(1));
                Assert.That(array[1].AsString(), Is.EqualTo("two"));
                Assert.That(array[2].AsBool(), Is.True);
                Assert.That(array[3].AsDouble(), Is.EqualTo(4.5));
            }
        }

        [Test]
        public void GetValue_ShouldReturnCorrectCustomTypes()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var posValue = doc.GetValue("Core", "pos");
                Assert.That(posValue.Type, Is.EqualTo(YiniType.Coord));
                var coord = posValue.AsCoord();
                Assert.That(coord.X, Is.EqualTo(10));
                Assert.That(coord.Y, Is.EqualTo(20));
                Assert.That(coord.Is3D, Is.False);

                var colorValue = doc.GetValue("Core", "color");
                Assert.That(colorValue.Type, Is.EqualTo(YiniType.Color));
                var color = colorValue.AsColor();
                Assert.That(color.R, Is.EqualTo(0));
                Assert.That(color.G, Is.EqualTo(255));
                Assert.That(color.B, Is.EqualTo(0));

                var assetValue = doc.GetValue("Core", "asset");
                Assert.That(assetValue.Type, Is.EqualTo(YiniType.Path));
                Assert.That(assetValue.AsPath(), Is.EqualTo("items/sword.mesh"));
            }
        }

        [Test]
        public void GetValue_InvalidCast_ShouldThrow()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var nameValue = doc.GetValue("Core", "name");
                Assert.Throws<InvalidCastException>(() => nameValue.AsInt());
                Assert.Throws<InvalidCastException>(() => nameValue.AsArray());
                Assert.Throws<InvalidCastException>(() => nameValue.AsCoord());
            }
        }
    }
}