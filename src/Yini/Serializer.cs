using System.Text;
using System.Linq;
using Yini.Model;

namespace Yini
{
    public class Serializer
    {
        public string Serialize(YiniDocument doc)
        {
            var sb = new StringBuilder();

            // Macros
            if (doc.Macros.Count > 0)
            {
                sb.AppendLine("[#define]");
                foreach(var kv in doc.Macros)
                {
                    sb.AppendLine($"{kv.Key} = {kv.Value}");
                }
                sb.AppendLine();
            }

            // Schemas
            foreach(var section in doc.Schemas.Values)
            {
                sb.AppendLine("[#schema]");
                SerializeSection(sb, section);
                sb.AppendLine();
            }

            // Sections
            foreach(var section in doc.Sections.Values)
            {
                SerializeSection(sb, section);
                sb.AppendLine();
            }

            return sb.ToString().Trim();
        }

        private void SerializeSection(StringBuilder sb, YiniSection section)
        {
            sb.Append($"[{section.Name}]");
            if (section.Parents.Count > 0)
            {
                sb.Append(" : ");
                sb.Append(string.Join(", ", section.Parents));
            }
            sb.AppendLine();

            foreach(var kv in section.Properties)
            {
                sb.AppendLine($"{kv.Key} = {kv.Value}");
            }

            foreach(var reg in section.Registry)
            {
                sb.AppendLine($"+= {reg}");
            }
        }
    }
}
