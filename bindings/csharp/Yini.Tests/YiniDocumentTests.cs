using NUnit.Framework;
using Yini;
using System;

namespace Yini.Tests
{
    public class YiniDocumentTests
    {
        [Test]
        public void ParseSimpleString_ShouldSucceed()
        {
            var source = "[Section]\nkey = value";
            YiniDocument doc = null;

            Assert.DoesNotThrow(() =>
            {
                doc = new YiniDocument(source);
            });

            Assert.IsNotNull(doc);
            doc.Dispose();
        }

        [Test]
        public void ParseInvalidString_ShouldThrow()
        {
            var source = "[Section\nkey = value"; // Invalid syntax
            Assert.Throws<Exception>(() =>
            {
                using (var doc = new YiniDocument(source))
                {
                    // Should not reach here
                }
            });
        }
    }
}