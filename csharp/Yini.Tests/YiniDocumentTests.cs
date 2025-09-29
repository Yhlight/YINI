using NUnit.Framework;

namespace YINI.Tests
{
    public class YiniDocumentTests
    {
        [Test]
        public void Parse_ValidInput_CanGetValues()
        {
            var yiniContent = @"
[TestSection]
string_val = ""hello""
int_val = 123
double_val = 45.6
bool_val_true = true
";

            using (var doc = YiniDocument.Parse(yiniContent))
            {
                Assert.That(doc.GetValue<string>("TestSection", "string_val"), Is.EqualTo("hello"));
                Assert.That(doc.GetValue<int>("TestSection", "int_val"), Is.EqualTo(123));
                Assert.That(doc.GetValue<double>("TestSection", "double_val"), Is.EqualTo(45.6));
                Assert.That(doc.GetValue<bool>("TestSection", "bool_val_true"), Is.True);
            }
        }

        [Test]
        public void SetValue_ValidInput_CanBeRetrieved()
        {
            var yiniContent = "[Data]";
            using (var doc = YiniDocument.Parse(yiniContent))
            {
                doc.SetValue("Data", "NewString", "world");
                doc.SetValue("Data", "NewInt", 999);

                Assert.That(doc.GetValue<string>("Data", "NewString"), Is.EqualTo("world"));
                Assert.That(doc.GetValue<int>("Data", "NewInt"), Is.EqualTo(999));
            }
        }

        [Test]
        public void Parse_InvalidInput_ThrowsException()
        {
            var yiniContent = "[UnclosedSection";

            var ex = Assert.Throws<InvalidOperationException>(() => YiniDocument.Parse(yiniContent));
            Assert.That(ex.Message, Contains.Substring("Failed to parse YINI content"));
            Assert.That(ex.Message, Contains.Substring("Expected ']' to close section header."));
        }
    }
}