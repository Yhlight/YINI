using Xunit;
using Yini.Core;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;

namespace Yini.Core.Tests
{
    public class CookedAssetTests
    {
        private const string TestYiniFileName = "csharp_cooked_test.yini";
        private const string TestYbinFileName = "csharp_cooked_test.ybin";

        private void CreateTestYiniFile()
        {
            string content = @"
[Stats]
level = 99
gold = 12345.67
is_pro = true
character_name = ""Jules""
";
            File.WriteAllText(TestYiniFileName, content);
        }

        private void CookTestFile()
        {
            // We need to invoke the native `yini` CLI to cook the file.
            // This assumes the CLI is built and accessible.
            // For a real CI setup, the path to the executable would be more robust.
            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "yini", // Now it should be in the same directory
                    Arguments = $"cook -o {TestYbinFileName} {TestYiniFileName}",
                    RedirectStandardOutput = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };
            process.Start();
            process.WaitForExit();
            Assert.Equal(0, process.ExitCode);
        }

        [Fact]
        public void LoadAndGetValuesFromCookedAsset()
        {
            CreateTestYiniFile();
            CookTestFile();

            Assert.True(File.Exists(TestYbinFileName));

            using (var asset = new YiniCookedAsset(TestYbinFileName))
            {
                // Assert correct values
                Assert.Equal(99, asset.GetInt("Stats", "level"));
                Assert.Equal(12345.67, asset.GetDouble("Stats", "gold"));
                Assert.True(asset.GetBool("Stats", "is_pro"));
                Assert.Equal("Jules", asset.GetString("Stats", "character_name"));

                // Assert non-existent values
                Assert.Null(asset.GetInt("Stats", "non_existent_key"));
                Assert.Null(asset.GetString("NonExistentSection", "level"));
            }

            File.Delete(TestYiniFileName);
            File.Delete(TestYbinFileName);
        }
    }
}
