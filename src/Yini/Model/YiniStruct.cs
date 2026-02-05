using System.Collections.Generic;
using System.Linq;

namespace Yini.Model
{
    // Struct: Fixed set of keys, performant, strict validation.
    // Syntax: {key: value} (No trailing comma)
    public class YiniStruct : YiniValue
    {
        public Dictionary<string, YiniValue> Fields { get; } = new Dictionary<string, YiniValue>();

        public override object GetRawValue() => Fields;

        public override YiniValue Clone()
        {
            var s = new YiniStruct();
            foreach(var kv in Fields) s.Fields[kv.Key] = kv.Value.Clone();
            CopySpanTo(s);
            return s;
        }

        public override string ToString() => "{" + string.Join(", ", Fields.Select(kv => $"{kv.Key}: {kv.Value}")) + "}";
    }
}
