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
    }
}
