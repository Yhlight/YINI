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
    }
}