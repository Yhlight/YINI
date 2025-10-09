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
    }
}