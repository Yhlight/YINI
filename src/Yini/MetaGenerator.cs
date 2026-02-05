using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Yini.Model;

namespace Yini
{
    public class MetaGenerator
    {
        public void Generate(YiniDocument doc, Stream outputStream)
        {
            // YMETA format:
            // Text-based (INI-like) or Binary?
            // Spec says: "YMETA uses .ymeta suffix. Caches all information to avoid repeated loading."
            // Since it's a cache, binary is better for speed.
            // But usually .meta files in engines are text (YAML/JSON).
            // Let's use the YINI Serializer format for now, effectively "flattening" the structure.
            // Wait, "avoid repeated loading" implies resolving includes and macros.
            // Our Compiler already does that.
            // So a YMETA file is basically a fully resolved YINI file (or YBIN).
            // But it might also contain separate metadata like "Source Hash" or "Dependencies".

            // Let's implement a text-based YMETA that includes:
            // 1. Dependency List (for build systems)
            // 2. The Resolved Content

            // Actually, usually .meta is sidecar.
            // "YMETA caches all information".
            // Let's write it as a serialized YINI file but with an extra [#meta] section.

            var writer = new StreamWriter(outputStream);
            writer.WriteLine("[#meta]");
            writer.WriteLine($"generated_at = \"{DateTime.UtcNow:O}\"");
            writer.WriteLine();

            var serializer = new Serializer();
            writer.Write(serializer.Serialize(doc));
            writer.Flush();
            // Do not dispose outputStream
        }
    }
}
