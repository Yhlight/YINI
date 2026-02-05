using System;
using System.IO;
using System.Collections.Generic;
using Godot;
using Yini;
using Yini.Model;

namespace Yini.Godot
{
    public partial class YiniResource : Resource
    {
        [Export] public Godot.Collections.Dictionary Data { get; set; }

        public static YiniResource Load(string path)
        {
            var res = new YiniResource();
            using (var file = FileAccess.Open(path, FileAccess.ModeFlags.Read))
            {
                byte[] bytes = file.GetBuffer((long)file.GetLength());
                var reader = new YiniBinaryReader();
                using (var ms = new MemoryStream(bytes))
                {
                    var doc = reader.Read(ms);
                    res.Data = ConvertDoc(doc);
                }
            }
            return res;
        }

        private static Godot.Collections.Dictionary ConvertDoc(YiniDocument doc)
        {
            var dict = new Godot.Collections.Dictionary();
            foreach (var kv in doc.Sections)
            {
                dict[kv.Key] = ConvertSection(kv.Value);
            }
            return dict;
        }

        private static Godot.Collections.Dictionary ConvertSection(YiniSection section)
        {
            var dict = new Godot.Collections.Dictionary();
            foreach (var kv in section.Properties)
            {
                dict[kv.Key] = ConvertValue(kv.Value);
            }
            return dict;
        }

        private static Variant ConvertValue(YiniValue val)
        {
            if (val is YiniInteger i) return i.Value;
            if (val is YiniFloat f) return f.Value;
            if (val is YiniBoolean b) return b.Value;
            if (val is YiniString s) return s.Value;
            if (val is YiniColor c) return new Color(c.R/255f, c.G/255f, c.B/255f, c.A/255f);
            if (val is YiniCoord co) return co.Is3D ? new Vector3(co.X, co.Y, co.Z) : new Vector2(co.X, co.Y);
            if (val is YiniArray arr)
            {
                var gdArr = new Godot.Collections.Array();
                foreach (var item in arr.Items) gdArr.Add(ConvertValue(item));
                return gdArr;
            }
            // Dyna -> String
            if (val is YiniDyna d) return d.Expression;
            return val.ToString();
        }
    }
}
