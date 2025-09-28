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
                // Test simple string
                var nameValue = doc.GetValue("Core", "name");
                Assert.That(nameValue, Is.Not.Null, "Name value should not be null");
                Assert.That(nameValue.Type, Is.EqualTo(YiniType.String), "Name type should be String");
                Assert.That(nameValue.AsString(), Is.EqualTo("YINI Engine"));

                // Test simple bool
                var enabledValue = doc.GetValue("Core", "enabled");
                Assert.That(enabledValue, Is.Not.Null, "Enabled value should not be null");
                Assert.That(enabledValue.Type, Is.EqualTo(YiniType.Bool), "Enabled type should be Bool");
                Assert.That(enabledValue.AsBool(), Is.True);

                // Test integer macro
                var factorValue = doc.GetValue("Core", "factor");
                Assert.That(factorValue, Is.Not.Null, "Factor value should not be null");
                Assert.That(factorValue.Type, Is.EqualTo(YiniType.Int), "Factor type should be Int");
                Assert.That(factorValue.AsInt(), Is.EqualTo(2));

                // Test double macro
                var versionValue = doc.GetValue("Core", "version");
                Assert.That(versionValue, Is.Not.Null, "Version value should not be null");
                Assert.That(versionValue.Type, Is.EqualTo(YiniType.Double), "Version type should be Double");
                Assert.That(versionValue.AsDouble(), Is.EqualTo(1.2));

                // Test arithmetic expression
                var rateValue = doc.GetValue("Values", "rate");
                Assert.That(rateValue, Is.Not.Null, "Rate value should not be null");
                Assert.That(rateValue.Type, Is.EqualTo(YiniType.Double), "Rate type should be Double");
                Assert.That(rateValue.AsDouble(), Is.EqualTo(25.0));

                // Test non-existent key
                var nonExistent = doc.GetValue("Core", "nonexistent");
                Assert.That(nonExistent, Is.Null, "Non-existent key should return null");
            }
        }

        [Test]
        public void GetValue_InvalidCast_ShouldThrow()
        {
            using(var doc = new YiniDocument(TestContent))
            {
                var nameValue = doc.GetValue("Core", "name");
                Assert.Throws<InvalidCastException>(() => nameValue.AsInt());
            }
        }
    }
}