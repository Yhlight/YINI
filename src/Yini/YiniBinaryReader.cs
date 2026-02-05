using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
using Yini.Model;

namespace Yini
{
    public class YiniBinaryReader
    {
        private const uint MAGIC = 0x59494E49; // YINI
        private const byte VERSION = 1;

        public YiniDocument Read(Stream stream)
        {
            var doc = new YiniDocument();
            using (var reader = new BinaryReader(stream, Encoding.UTF8, true))
            {
                uint magic = reader.ReadUInt32();
                if (magic != MAGIC) throw new Exception("Invalid YBIN file");
                byte version = reader.ReadByte();
                if (version > VERSION) throw new Exception($"Unsupported YBIN version: {version}");

                int sectionCount = reader.ReadInt32();
                for (int i = 0; i < sectionCount; i++)
                {
                    var section = ReadSection(reader);
                    doc.Sections[section.Name] = section;
                }
            }
            return doc;
        }

        private YiniSection ReadSection(BinaryReader reader)
        {
            string name = reader.ReadString();
            var section = new YiniSection(name);

            int parentCount = reader.ReadInt32();
            for(int i=0; i<parentCount; i++) section.Parents.Add(reader.ReadString());

            int propCount = reader.ReadInt32();
            for(int i=0; i<propCount; i++)
            {
                string key = reader.ReadString();
                section.Properties[key] = ReadValue(reader);
            }

            int regCount = reader.ReadInt32();
            for(int i=0; i<regCount; i++) section.Registry.Add(ReadValue(reader));

            return section;
        }

        private YiniValue ReadValue(BinaryReader reader)
        {
            byte type = reader.ReadByte();
            switch(type)
            {
                case 1: return new YiniInteger(reader.ReadInt32());
                case 2: return new YiniFloat(reader.ReadSingle());
                case 3: return new YiniBoolean(reader.ReadBoolean());
                case 4: return new YiniString(reader.ReadString());
                case 5: return new YiniColor(reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32(), reader.ReadInt32());
                case 6:
                    bool is3d = reader.ReadBoolean();
                    float x = reader.ReadSingle();
                    float y = reader.ReadSingle();
                    float z = reader.ReadSingle();
                    return is3d ? new YiniCoord(x,y,z) : new YiniCoord(x,y);
                case 7: return new YiniPath(reader.ReadString());
                case 8:
                    int count = reader.ReadInt32();
                    var list = new List<YiniValue>();
                    for(int i=0; i<count; i++) list.Add(ReadValue(reader));
                    return new YiniArray(list);
                case 9:
                    count = reader.ReadInt32();
                    var llist = new List<YiniValue>();
                    for(int i=0; i<count; i++) llist.Add(ReadValue(reader));
                    return new YiniList(llist);
                case 10:
                    count = reader.ReadInt32();
                    var slist = new List<YiniValue>();
                    for(int i=0; i<count; i++) slist.Add(ReadValue(reader));
                    return new YiniSet(slist);
                case 11:
                    count = reader.ReadInt32();
                    var map = new YiniMap();
                    for(int i=0; i<count; i++) map.Items[reader.ReadString()] = ReadValue(reader);
                    return map;
                case 12:
                    var rType = (ReferenceType)reader.ReadInt32();
                    var rRef = reader.ReadString();
                    return new YiniReference(rRef, rType);
                default:
                    throw new Exception($"Unknown value type: {type}");
            }
        }
    }
}
