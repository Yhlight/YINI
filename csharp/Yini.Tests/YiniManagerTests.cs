using NUnit.Framework;
using YINI;
using System;
using System.IO;

namespace Yini.Tests
{
    [TestFixture]
    public class YiniManagerTests
    {
        private const string TestFileName = "manager_test.yini";
        private const string TestFileCopyName = "manager_test_copy.yini";

        [SetUp]
        public void SetUp()
        {
            // The manager modifies the file, so we run tests on a copy.
            if (File.Exists(TestFileName))
            {
                File.Copy(TestFileName, TestFileCopyName, true);
            }
        }

        [TearDown]
        public void TearDown()
        {
            // Clean up the copied file and any generated meta file.
            if (File.Exists(TestFileCopyName))
            {
                File.Delete(TestFileCopyName);
            }
            string metaFile = Path.ChangeExtension(TestFileCopyName, ".ymeta");
            if (File.Exists(metaFile))
            {
                File.Delete(metaFile);
            }
            string ymetaBackup = metaFile + ".bak1";
            if (File.Exists(ymetaBackup))
            {
                File.Delete(ymetaBackup);
            }
        }

        [Test]
        public void YiniManager_LoadsFileAndGetsInitialValues()
        {
            using (var manager = new YiniManager(TestFileCopyName))
            {
                Assert.That(manager.IsLoaded, Is.True);
                var doc = manager.Document;

                var fullscreen = doc.GetValue("Settings", "fullscreen");
                Assert.That(fullscreen, Is.Not.Null);
                Assert.That(fullscreen.Type, Is.EqualTo(YiniType.Dyna));
                Assert.That(fullscreen.AsDyna().AsBool(), Is.True);

                var volume = doc.GetValue("Settings", "volume");
                Assert.That(volume, Is.Not.Null);
                Assert.That(volume.Type, Is.EqualTo(YiniType.Dyna));
                Assert.That(volume.AsDyna().AsInt(), Is.EqualTo(100));

                var score = doc.GetValue("Player", "score");
                Assert.That(score, Is.Not.Null);
                Assert.That(score.Type, Is.EqualTo(YiniType.Dyna));
                Assert.That(score.AsDyna().AsInt(), Is.EqualTo(0));
            }
        }

        [Test]
        public void YiniManager_SetValue_UpdatesDynaValueAndPersists()
        {
            // Phase 1: Create manager, set values, and dispose to trigger write-back.
            using (var manager = new YiniManager(TestFileCopyName))
            {
                Assert.That(manager.IsLoaded, Is.True);

                // Update the values
                manager.SetValue("Settings", "fullscreen", false);
                manager.SetValue("Player", "score", 999);
            } // Dispose is called here, which should write the changes to the .yini file.

            // Phase 2: Verify the file content was updated.
            // The C++ implementation of write-back doesn't exist yet, so this will fail.
            // This test is designed to drive the implementation.
            string fileContent = File.ReadAllText(TestFileCopyName);
            StringAssert.Contains("fullscreen = Dyna(false)", fileContent);
            StringAssert.Contains("score = Dyna(999)", fileContent);

            // Phase 3: Create a new manager to ensure the updated values are loaded correctly.
            using (var manager2 = new YiniManager(TestFileCopyName))
            {
                var doc = manager2.Document;
                var fullscreen = doc.GetValue("Settings", "fullscreen");
                Assert.That(fullscreen.AsDyna().AsBool(), Is.False);

                var score = doc.GetValue("Player", "score");
                Assert.That(score.AsDyna().AsInt(), Is.EqualTo(999));
            }
        }
    }
}