using System;
using System.Collections.Generic;
using System.Linq;
using Yini.Model;

namespace Yini
{
    public static class LspHelper
    {
        public static YiniSection FindSectionAt(YiniDocument doc, int line, int col)
        {
            // Simple heuristic: Find section where line is between NameSpan and next section
            // But sections don't store end line.
            // However, properties have locations. We can check bounds.
            // Better: Iterate all sections, find one that "contains" or is closest above.
            // Assuming Parser adds sections in order? Dictionary doesn't guarantee order.
            // But we can check NameSpan.

            YiniSection candidate = null;

            foreach (var section in doc.Sections.Values)
            {
                if (section.NameSpan == null) continue; // #define/#include might not have NameSpan if implicit? No, Parser sets it for normal sections.

                // If cursor is on the header line
                if (section.NameSpan.Line == line) return section;

                // If cursor is below header
                if (section.NameSpan.Line < line)
                {
                    if (candidate == null || section.NameSpan.Line > candidate.NameSpan.Line)
                    {
                        candidate = section;
                    }
                }
            }
            return candidate;
        }

        public static string GetHover(YiniDocument doc, int line, int col)
        {
            var section = FindSectionAt(doc, line, col);
            if (section == null) return null;

            // Check if hovering a Key
            foreach (var kv in section.KeyLocations)
            {
                var span = kv.Value;
                if (span.Line == line && col >= span.Column && col <= span.Column + kv.Key.Length)
                {
                    // Found key. Look up schema.
                    return GetSchemaHover(doc, section.Name, kv.Key);
                }
            }

            // Check if hovering Section Header
            if (section.NameSpan.Line == line && col >= section.NameSpan.Column && col <= section.NameSpan.Column + section.Name.Length)
            {
                return $"**Section** `[{section.Name}]`";
            }

            return null;
        }

        private static string GetSchemaHover(YiniDocument doc, string sectionName, string key)
        {
            if (doc.Schemas.TryGetValue(sectionName, out var schema))
            {
                if (schema.Properties.TryGetValue(key, out var val) && val is YiniSchemaDefinition def)
                {
                    var sb = new System.Text.StringBuilder();
                    sb.AppendLine($"**Property** `{key}`");
                    sb.AppendLine($"*   **Type**: `{def.TypeName}`");
                    sb.AppendLine($"*   **Required**: `{(def.Requirement == SchemaRequirement.Required ? "Yes" : "No")}`");
                    if (def.DefaultValue != null) sb.AppendLine($"*   **Default**: `{def.DefaultValue}`");
                    if (def.Min != null) sb.AppendLine($"*   **Min**: `{def.Min}`");
                    if (def.Max != null) sb.AppendLine($"*   **Max**: `{def.Max}`");
                    return sb.ToString();
                }
            }
            return $"**Property** `{key}` (No Schema)";
        }

        public static List<string> GetCompletion(YiniDocument doc, int line, int col)
        {
            var section = FindSectionAt(doc, line, col);
            if (section == null) return new List<string> { "true", "false", "Color", "Coord" };

            var items = new List<string>();

            // Suggest keys from schema that are NOT in section
            if (doc.Schemas.TryGetValue(section.Name, out var schema))
            {
                foreach (var key in schema.Properties.Keys)
                {
                    if (!section.Properties.ContainsKey(key))
                    {
                        items.Add(key);
                    }
                }
            }

            return items;
        }
    }
}
