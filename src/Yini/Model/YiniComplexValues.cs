using System.Collections.Generic;
using System.Linq;

namespace Yini.Model
{
    public class YiniArray : YiniValue
    {
        public List<YiniValue> Items { get; } = new List<YiniValue>();
        public YiniArray(IEnumerable<YiniValue> items = null)
        {
            if (items != null) Items.AddRange(items);
        }
        public override object GetRawValue() => Items;
        public override YiniValue Clone() { var c = new YiniArray(Items.Select(i => i.Clone())); CopySpanTo(c); return c; }
        public override string ToString() => $"[{string.Join(", ", Items)}]";
    }

    public class YiniList : YiniValue
    {
        public List<YiniValue> Items { get; } = new List<YiniValue>();
        public YiniList(IEnumerable<YiniValue> items = null)
        {
            if (items != null) Items.AddRange(items);
        }
        public override object GetRawValue() => Items;
        public override YiniValue Clone() { var c = new YiniList(Items.Select(i => i.Clone())); CopySpanTo(c); return c; }
        public override string ToString() => $"List({string.Join(", ", Items)})";
    }

    public class YiniSet : YiniValue
    {
        public List<YiniValue> Elements { get; } = new List<YiniValue>();

        public YiniSet(IEnumerable<YiniValue> items = null)
        {
            if (items != null) Elements.AddRange(items);
        }
        public override object GetRawValue() => Elements;
        public override YiniValue Clone() { var c = new YiniSet(Elements.Select(i => i.Clone())); CopySpanTo(c); return c; }
        public override string ToString() => $"({string.Join(", ", Elements)})";
    }

    public class YiniMap : YiniValue
    {
        public Dictionary<string, YiniValue> Items { get; } = new Dictionary<string, YiniValue>();
        public override object GetRawValue() => Items;
        public override YiniValue Clone()
        {
            var map = new YiniMap();
            foreach(var kv in Items) map.Items[kv.Key] = kv.Value.Clone();
            CopySpanTo(map);
            return map;
        }
        public override string ToString() => "{" + string.Join(", ", Items.Select(kv => $"{kv.Key}: {kv.Value}")) + "}";
    }

    public class YiniColor : YiniValue
    {
        public int R { get; set; }
        public int G { get; set; }
        public int B { get; set; }
        public int A { get; set; } = 255;

        public YiniColor(int r, int g, int b, int a = 255)
        {
            R = r; G = g; B = b; A = a;
        }
        public override object GetRawValue() => this;
        public override YiniValue Clone() { var c = new YiniColor(R, G, B, A); CopySpanTo(c); return c; }
        public override string ToString() => $"Color({R}, {G}, {B}, {A})";
    }

    public class YiniCoord : YiniValue
    {
        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }
        public bool Is3D { get; set; }

        public YiniCoord(float x, float y)
        {
            X = x; Y = y; Is3D = false;
        }
        public YiniCoord(float x, float y, float z)
        {
            X = x; Y = y; Z = z; Is3D = true;
        }
        public override object GetRawValue() => this;
        public override YiniValue Clone() { var c = Is3D ? new YiniCoord(X, Y, Z) : new YiniCoord(X, Y); CopySpanTo(c); return c; }
        public override string ToString() => Is3D ? $"Coord({X}, {Y}, {Z})" : $"Coord({X}, {Y})";
    }

    public class YiniPath : YiniValue
    {
        public string Path { get; set; }
        public YiniPath(string path) => Path = path;
        public override object GetRawValue() => Path;
        public override YiniValue Clone() { var c = new YiniPath(Path); CopySpanTo(c); return c; }
        public override string ToString() => $"Path(\"{Path}\")";
    }

    public enum ReferenceType
    {
        Environment, // ${...}
        CrossSection, // @{...}
        Macro, // @name
        Localization // @i18n:key
    }

    public class YiniReference : YiniValue
    {
        public string Reference { get; set; }
        public ReferenceType Type { get; set; }

        public YiniReference(string reference, ReferenceType type)
        {
            Reference = reference;
            Type = type;
        }
        public override object GetRawValue() => $"Ref({Type}:{Reference})";
        public override YiniValue Clone() { var c = new YiniReference(Reference, Type); CopySpanTo(c); return c; }
        public override string ToString()
        {
            switch(Type)
            {
                case ReferenceType.Macro: return $"@{Reference}";
                case ReferenceType.Environment: return $"${{{Reference}}}";
                case ReferenceType.CrossSection: return $"@{{{Reference}}}";
                default: return GetRawValue().ToString();
            }
        }
    }
}
