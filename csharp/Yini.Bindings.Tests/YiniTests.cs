using NUnit.Framework;
using System.IO;

namespace Yini.Bindings.Tests
{
    public class YiniTests
    {
        private const string TestFileName = "test.yini";

        [SetUp]
        public void Setup()
        {
            // Create a dummy YINI file for testing
            using (StreamWriter sw = File.CreateText(TestFileName))
            {
                sw.WriteLine("[Settings]");
                sw.WriteLine("name = Jules");
                sw.WriteLine("level = 99");
                sw.WriteLine("rating = 9.5");
                sw.WriteLine("enabled = true");
            }
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
        public void LoadAndGetValues()
        {
            // This assumes the native library is in the same directory or in the system's search path.
            // We'll need to configure the build to copy the native library to the test output directory.
            using (var yini = new Yini(TestFileName))
            {
                Assert.That(yini.GetString("Settings", "name"), Is.EqualTo("Jules"));
                Assert.That(yini.GetInt("Settings", "level"), Is.EqualTo(99));
                Assert.That(yini.GetDouble("Settings", "rating"), Is.EqualTo(9.5));
                Assert.That(yini.GetBool("Settings", "enabled"), Is.True);
            }
        }
    }
}