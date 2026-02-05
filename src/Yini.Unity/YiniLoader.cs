using System;
using System.IO;
using System.Collections.Generic;
using UnityEngine;
using Yini;
using Yini.Model;

namespace Yini.Unity
{
    public class YiniLoader : MonoBehaviour
    {
        public TextAsset YBinFile;

        public Dictionary<string, object> Load()
        {
            if (YBinFile == null) return null;

            var reader = new YiniBinaryReader();
            using (var ms = new MemoryStream(YBinFile.bytes))
            {
                var doc = reader.Read(ms);
                return ConvertDoc(doc);
            }
        }

        private Dictionary<string, object> ConvertDoc(YiniDocument doc)
        {
            var result = new Dictionary<string, object>();
            foreach (var kv in doc.Sections)
            {
                result[kv.Key] = ConvertSection(kv.Value);
            }
            return result;
        }

        private Dictionary<string, object> ConvertSection(YiniSection section)
        {
            var dict = new Dictionary<string, object>();
            foreach (var kv in section.Properties)
            {
                dict[kv.Key] = ConvertValue(kv.Value);
            }
            return dict;
        }

        private object ConvertValue(YiniValue val)
        {
            if (val is YiniInteger i) return i.Value;
            if (val is YiniFloat f) return f.Value;
            if (val is YiniBoolean b) return b.Value;
            if (val is YiniString s) return s.Value;
            if (val is YiniColor c) return new Color32((byte)c.R, (byte)c.G, (byte)c.B, (byte)c.A);
            if (val is YiniCoord co) return co.Is3D ? new Vector3(co.X, co.Y, co.Z) : new Vector2(co.X, co.Y);
            if (val is YiniArray arr)
            {
                var list = new List<object>();
                foreach (var item in arr.Items) list.Add(ConvertValue(item));
                return list;
            }
            // ... Map, List, Set, Dyna
            if (val is YiniDyna d) return d.Expression; // Raw string
            return val.ToString();
        }
    }
}
