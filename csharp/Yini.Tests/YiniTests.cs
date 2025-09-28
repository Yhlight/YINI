using NUnit.Framework;
using YINI;

namespace Yini.Tests
{
    public class YiniTests
    {
        [Test]
        public void ParseDocument_ShouldReturnCorrectSectionCount()
        {
            var content = "[Section1]\nkey=val\n[Section2]\nkey=val";
            using (var doc = new YiniDocument(content))
            {
                Assert.That(doc.SectionCount, Is.EqualTo(2));
            }
        }

        [Test]
        public void GetSectionName_ShouldReturnCorrectName()
        {
            var content = "[Section1]\nkey=val\n[MySection]\nkey=val";
            using (var doc = new YiniDocument(content))
            {
                Assert.That(doc.GetSectionName(0), Is.EqualTo("Section1"));
                Assert.That(doc.GetSectionName(1), Is.EqualTo("MySection"));
            }
        }
    }
}