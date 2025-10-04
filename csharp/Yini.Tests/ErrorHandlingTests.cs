using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Yini;
using System.Collections.Generic;

namespace Yini.Tests
{
    [YiniBindable]
    public partial class ErrorTestBindable
    {
        public string Name { get; set; } = "DefaultName";
        public int Level { get; set; } = 1;
        public bool IsActive { get; set; }
    }

    [TestClass]
    public class ErrorHandlingTests
    {
        private const string TestFileName = "error_test.yini";
        private YiniManager _manager = null!;

        [TestInitialize]
        public void Setup()
        {
            // This manager will be used for most tests
            var content = @"
[player]
name = Jules
level = 99
isactive = not_a_bool
";
            File.WriteAllText(TestFileName, content);
            _manager = new YiniManager();
            _manager.Load(TestFileName);
        }

        [TestCleanup]
        public void Cleanup()
        {
            _manager?.Dispose();
            if (File.Exists(TestFileName))
            {
                File.Delete(TestFileName);
            }
        }

        [TestMethod]
        public void ReflectionBind_WithMissingSection_ReturnsDefaultObject()
        {
            // Act
            var model = _manager.Bind<ErrorTestBindable>("non_existent_section");

            // Assert
            Assert.IsNotNull(model);
            Assert.AreEqual("DefaultName", model.Name);
            Assert.AreEqual(1, model.Level);
        }

        [TestMethod]
        public void ReflectionBind_WithMissingKey_KeepsDefaultValue()
        {
            // Arrange: Create a file that is missing the 'level' key
            var content = @"
[player]
name = Jules
";
            using(var manager = new YiniManager())
            {
                File.WriteAllText(TestFileName, content);
                manager.Load(TestFileName);

                // Act
                var model = manager.Bind<ErrorTestBindable>("player");

                // Assert
                Assert.IsNotNull(model);
                Assert.AreEqual("Jules", model.Name); // Name should be bound
                Assert.AreEqual(1, model.Level);     // Level should keep its default
            }
        }

        [TestMethod]
        public void ReflectionBind_WithTypeMismatch_ThrowsException()
        {
            // Assert
            Assert.ThrowsException<System.ArgumentException>(() =>
            {
                // Act
                _manager.Bind<ErrorTestBindable>("player");
            }, "Binding 'isactive' (a bool) to 'not_a_bool' (a string) should fail.");
        }

        [TestMethod]
        public void SourceGenBind_WithMissingSection_KeepsDefaultValues()
        {
            // Arrange
            var model = new ErrorTestBindable();

            // Act
            model.BindFromYini(_manager, "non_existent_section");

            // Assert
            Assert.AreEqual("DefaultName", model.Name);
            Assert.AreEqual(1, model.Level);
        }

        [TestMethod]
        public void SourceGenBind_WithMissingKey_KeepsDefaultValue()
        {
            // Arrange
            var content = @"
[player]
name = Jules
";
            using (var manager = new YiniManager())
            {
                File.WriteAllText(TestFileName, content);
                manager.Load(TestFileName);
                var model = new ErrorTestBindable();

                // Act
                model.BindFromYini(manager, "player");

                // Assert
                Assert.AreEqual("Jules", model.Name); // Name should be bound
                Assert.AreEqual(1, model.Level);     // Level should keep its default
            }
        }
    }
}