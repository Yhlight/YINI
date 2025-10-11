using Xunit;
using Yini.Core;
using System;

namespace Yini.Core.Tests
{
    public class ErrorHandlingTests
    {
        [Fact]
        public void CreateFromFile_NonExistentFile_ThrowsYiniException()
        {
            // Arrange
            var nonExistentFile = "non_existent_file.yini";

            // Act & Assert
            var exception = Assert.Throws<YiniException>(() => new YiniConfig(nonExistentFile));
            Assert.Contains("Could not open file", exception.Message);
        }

        [Fact]
        public void CreateFromFile_ParseError_ThrowsYiniException()
        {
            // Arrange
            var invalidFile = "parse_error.yini";
            System.IO.File.WriteAllText(invalidFile, "[Section]\nkey = value"); // Missing quotes

            // Act & Assert
            var exception = Assert.Throws<YiniException>(() => new YiniConfig(invalidFile));
            Assert.Contains("Error at line 2", exception.Message);

            // Cleanup
            System.IO.File.Delete(invalidFile);
        }
    }
}
