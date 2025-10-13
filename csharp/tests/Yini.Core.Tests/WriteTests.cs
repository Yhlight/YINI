using System;
using System.IO;
using Xunit;
using Yini.Core;

namespace Yini.Core.Tests
{
    public class WriteTests
    {
        [Fact]
        public void CanCreateSetAndSaveValues()
        {
            var tempFile = Path.GetTempFileName();
            try
            {
                // Arrange: Create a new config
                using (var config = new YiniConfig())
                {
                    // Act: Set values
                    config.SetValue("Output.width", 1920);
                    config.SetValue("Output.height", 1080.5); // Use double
                    config.SetValue("Settings.fullscreen", true);
                    config.SetValue("Player.name", "Jules");

                    // Save to file
                    config.Save(tempFile);
                }

                // Assert: Load the file back and verify values
                using (var loadedConfig = new YiniConfig(tempFile))
                {
                    Assert.Equal(1920, loadedConfig.GetInt("Output.width"));
                    Assert.Equal(1080.5, loadedConfig.GetDouble("Output.height"));
                    Assert.True(loadedConfig.GetBool("Settings.fullscreen"));
                    Assert.Equal("Jules", loadedConfig.GetString("Player.name"));
                }
            }
            finally
            {
                // Cleanup
                if (File.Exists(tempFile))
                {
                    File.Delete(tempFile);
                }
            }
        }
    }
}
