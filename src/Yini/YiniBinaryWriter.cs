using System;
using System.IO;
using System.Linq;
using System.Text;
using Yini.Model;

namespace Yini
{
    public class YiniBinaryWriter
    {
        private const uint MAGIC = 0x59494E49; // YINI
        private const byte VERSION = 1;

        public void Write(YiniDocument doc, Stream stream)
        {
            using (var writer = new BinaryWriter(stream, Encoding.UTF8, true))
            {
                writer.Write(MAGIC);
                writer.Write(VERSION);

                // Sections
                writer.Write(doc.Sections.Count);
                foreach(var kv in doc.Sections)
                {
                    WriteSection(writer, kv.Value);
                }
            }
        }

        private void WriteSection(BinaryWriter writer, YiniSection section)
        {
            writer.Write(section.Name);

            // Parents
            writer.Write(section.Parents.Count);
            foreach(var p in section.Parents) writer.Write(p);

            // Properties
            writer.Write(section.Properties.Count);
            foreach(var kv in section.Properties)
            {
                writer.Write(kv.Key);
                WriteValue(writer, kv.Value);
            }

            // Registry
            writer.Write(section.Registry.Count);
            foreach(var item in section.Registry)
            {
                WriteValue(writer, item);
            }
        }

        private void WriteValue(BinaryWriter writer, YiniValue value)
        {
            if (value is YiniInteger i) { writer.Write((byte)1); writer.Write(i.Value); }
            else if (value is YiniFloat f) { writer.Write((byte)2); writer.Write(f.Value); }
            else if (value is YiniBoolean b) { writer.Write((byte)3); writer.Write(b.Value); }
            else if (value is YiniString s) { writer.Write((byte)4); writer.Write(s.Value); }
            else if (value is YiniColor c)
            {
                writer.Write((byte)5);
                writer.Write(c.R); writer.Write(c.G); writer.Write(c.B); writer.Write(c.A);
            }
            else if (value is YiniCoord coord)
            {
                writer.Write((byte)6);
                writer.Write(coord.Is3D); writer.Write(coord.X); writer.Write(coord.Y); writer.Write(coord.Z);
            }
            else if (value is YiniPath p) { writer.Write((byte)7); writer.Write(p.Path); }
            else if (value is YiniArray arr)
            {
                writer.Write((byte)8);
                writer.Write(arr.Items.Count);
                foreach(var item in arr.Items) WriteValue(writer, item);
            }
            else if (value is YiniList l)
            {
                writer.Write((byte)9);
                writer.Write(l.Items.Count);
                foreach(var item in l.Items) WriteValue(writer, item);
            }
            else if (value is YiniSet set)
            {
                writer.Write((byte)10);
                writer.Write(set.Elements.Count);
                foreach(var item in set.Elements) WriteValue(writer, item);
            }
            else if (value is YiniMap map)
            {
                writer.Write((byte)11);
                writer.Write(map.Items.Count);
                foreach(var kv in map.Items) { writer.Write(kv.Key); WriteValue(writer, kv.Value); }
            }
            else if (value is YiniReference r)
            {
                 // Typically binary format is "cooked" (resolved), but if we allow refs:
                 writer.Write((byte)12);
                 writer.Write((int)r.Type);
                 writer.Write(r.Reference);
            }
            else if (value is YiniDyna d)
            {
                writer.Write((byte)13);
                writer.Write(d.Expression);
            }
            else
            {
                throw new Exception($"Unsupported type for binary serialization: {value.GetType().Name}");
            }
        }
    }
}
