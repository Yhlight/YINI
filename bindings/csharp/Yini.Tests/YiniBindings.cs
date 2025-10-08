using NUnit.Framework;
using System.IO;
using System;

namespace YINI.Tests
{
    [TestFixture]
    public class YiniBindings
    {
        private const string TestYiniContent = @"
[Settings]
width = 1920
height = 1080
fullscreen = true
title = ""Test Game""

[Resources]
textures = [""player.png"", ""enemy.png""]
";

        [Test]
        public void ParseFromString_And_GetValues()
        {
            using (var parser = new Parser(TestYiniContent))
            {
                Assert.IsTrue(parser.Parse(), $"Parse failed with error: {parser.GetError()}");

                var settings = parser.GetSection("Settings");
                Assert.IsNotNull(settings);

                var width = settings.GetValue("width");
                Assert.IsNotNull(width);
                Assert.AreEqual(1920, width.AsInteger());

                var title = settings.GetValue("title");
                Assert.IsNotNull(title);
                Assert.AreEqual("Test Game", title.AsString());

                var fullscreen = settings.GetValue("fullscreen");
                Assert.IsNotNull(fullscreen);
                Assert.IsTrue(fullscreen.AsBoolean());
            }
        }

        [Test]
        public void GetSectionNames_And_Keys()
        {
            using (var parser = new Parser(TestYiniContent))
            {
                Assert.IsTrue(parser.Parse());

                var sectionNames = parser.GetSectionNames();
                Assert.AreEqual(2, sectionNames.Length);
                CollectionAssert.AreEquivalent(new[] { "Settings", "Resources" }, sectionNames);

                var settings = parser.GetSection("Settings");
                Assert.IsNotNull(settings);
                var settingKeys = settings.GetKeys();
                CollectionAssert.AreEquivalent(new[] { "width", "height", "fullscreen", "title" }, settingKeys);
            }
        }

        [Test]
        public void GetArrayValue()
        {
            using (var parser = new Parser(TestYiniContent))
            {
                Assert.IsTrue(parser.Parse());

                var resources = parser.GetSection("Resources");
                Assert.IsNotNull(resources);

                var textures = resources.GetValue("textures");
                Assert.IsNotNull(textures);
                Assert.AreEqual(ValueType.Array, textures.GetValueType());
                Assert.AreEqual(2, textures.GetArraySize());

                var firstTexture = textures.GetArrayElement(0);
                Assert.IsNotNull(firstTexture);
                Assert.AreEqual("player.png", firstTexture.AsString());

                var secondTexture = textures.GetArrayElement(1);
                Assert.IsNotNull(secondTexture);
                Assert.AreEqual("enemy.png", secondTexture.AsString());
            }
        }

        [Test]
        public void ParseFromFile_And_GetValues()
        {
            string tempFile = Path.GetTempFileName();
            File.WriteAllText(tempFile, TestYiniContent);

            try
            {
                using (var parser = Parser.FromFile(tempFile))
                {
                    Assert.IsTrue(parser.Parse(), $"Parse from file failed: {parser.GetError()}");
                    var settings = parser.GetSection("Settings");
                    Assert.IsNotNull(settings);
                    Assert.AreEqual(1920, settings.GetValue("width").AsInteger());
                }
            }
            finally
            {
                File.Delete(tempFile);
            }
        }

        [Test]
        public void ParseError_Is_Handled()
        {
            string invalidContent = "[Section\nkey=value"; // Missing closing bracket
            using (var parser = new Parser(invalidContent))
            {
                Assert.IsFalse(parser.Parse());
                Assert.IsNotEmpty(parser.GetError());
                Console.WriteLine($"Caught expected error: {parser.GetError()}");
            }
        }
    }
}