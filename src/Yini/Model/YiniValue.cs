using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace Yini.Model
{
    public abstract class YiniValue
    {
        public SourceSpan Span { get; set; }
        public abstract object GetRawValue();
        public abstract YiniValue Clone();
        public override string ToString() => GetRawValue()?.ToString() ?? "null";

        protected void CopySpanTo(YiniValue other)
        {
            other.Span = Span;
        }
    }

    public class YiniInteger : YiniValue
    {
        public int Value { get; set; }
        public YiniInteger(int value) => Value = value;
        public override object GetRawValue() => Value;
        public override YiniValue Clone() { var c = new YiniInteger(Value); CopySpanTo(c); return c; }
    }

    public class YiniFloat : YiniValue
    {
        public float Value { get; set; }
        public YiniFloat(float value) => Value = value;
        public override object GetRawValue() => Value;
        public override YiniValue Clone() { var c = new YiniFloat(Value); CopySpanTo(c); return c; }
    }

    public class YiniBoolean : YiniValue
    {
        public bool Value { get; set; }
        public YiniBoolean(bool value) => Value = value;
        public override object GetRawValue() => Value;
        public override YiniValue Clone() { var c = new YiniBoolean(Value); CopySpanTo(c); return c; }
        public override string ToString() => Value.ToString().ToLower();
    }

    public class YiniString : YiniValue
    {
        public string Value { get; set; }
        public YiniString(string value) => Value = value;
        public override object GetRawValue() => Value;
        public override YiniValue Clone() { var c = new YiniString(Value); CopySpanTo(c); return c; }
        public override string ToString() => $"\"{Value}\"";
    }
}
