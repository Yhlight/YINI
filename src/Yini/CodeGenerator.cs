using System;
using System.IO;
using System.Text;
using System.Linq;
using Yini.Model;

namespace Yini
{
    public class CodeGenerator
    {
        public string GenerateCSharp(YiniDocument doc, string namespaceName, string className)
        {
            var sb = new StringBuilder();
            sb.AppendLine("using System;");
            sb.AppendLine("using System.Collections.Generic;");
            sb.AppendLine("using Yini.Model;");
            sb.AppendLine("using Yini;");
            sb.AppendLine();
            sb.AppendLine($"namespace {namespaceName}");
            sb.AppendLine("{");
            sb.AppendLine($"    public class {className}");
            sb.AppendLine("    {");

            // Properties
            foreach(var sectionKv in doc.Schemas)
            {
                sb.AppendLine($"        public {sectionKv.Key}Config {sectionKv.Key} {{ get; set; }} = new {sectionKv.Key}Config();");
            }
            sb.AppendLine();

            // Load Method
            sb.AppendLine("        public void Load(YiniDocument doc)");
            sb.AppendLine("        {");
            sb.AppendLine("            var evaluator = new Evaluator(new Compiler()); // Basic context or passed in?");
            sb.AppendLine("            // Assuming doc is already compiled/resolved usually.");
            foreach(var sectionKv in doc.Schemas)
            {
                sb.AppendLine($"            if (doc.Sections.ContainsKey(\"{sectionKv.Key}\"))");
                sb.AppendLine($"                {sectionKv.Key}.Load(doc.Sections[\"{sectionKv.Key}\"]);");
            }
            sb.AppendLine("        }");

            // Inner Classes
            foreach(var sectionKv in doc.Schemas)
            {
                GenerateSectionClass(sb, sectionKv.Key, sectionKv.Value);
            }

            sb.AppendLine("    }");
            sb.AppendLine("}");
            return sb.ToString();
        }

        private void GenerateSectionClass(StringBuilder sb, string name, YiniSection schema)
        {
            sb.AppendLine($"    public class {name}Config");
            sb.AppendLine("    {");

            // Properties
            foreach(var prop in schema.Properties)
            {
                if (prop.Value is YiniSchemaDefinition def)
                {
                    string type = MapToCSharpType(def.TypeName);
                    sb.AppendLine($"        public {type} {prop.Key} {{ get; set; }}");
                }
            }
            sb.AppendLine();

            // Load Method
            sb.AppendLine("        public void Load(YiniSection section)");
            sb.AppendLine("        {");
            foreach(var prop in schema.Properties)
            {
                 if (prop.Value is YiniSchemaDefinition def)
                {
                    string type = MapToCSharpType(def.TypeName);
                    // Use helper to convert
                    sb.AppendLine($"            if (section.Properties.ContainsKey(\"{prop.Key}\"))");
                    sb.AppendLine($"                this.{prop.Key} = ({type})ConvertValue(section.Properties[\"{prop.Key}\"], typeof({type}));");
                }
            }
            sb.AppendLine("        }");

            // Helper (embedded to avoid dependency issues if library changes)
            sb.AppendLine(@"
        private object ConvertValue(YiniValue val, Type targetType)
        {
            if (val is YiniInteger i)
            {
                if (targetType == typeof(int)) return i.Value;
                if (targetType == typeof(float)) return (float)i.Value;
            }
            if (val is YiniFloat f && targetType == typeof(float)) return f.Value;
            if (val is YiniBoolean b && targetType == typeof(bool)) return b.Value;
            if (val is YiniString s && targetType == typeof(string)) return s.Value;
            // Add other conversions (Color, etc)
            return default;
        }");

            sb.AppendLine("    }");
            sb.AppendLine();
        }

        private string MapToCSharpType(string yiniType)
        {
            if (yiniType.StartsWith("array["))
            {
                // Extract inner type: array[int] -> int
                string inner = yiniType.Substring(6, yiniType.Length - 7);
                return $"List<{MapToCSharpType(inner)}>";
            }

            switch(yiniType)
            {
                case "int": return "int";
                case "float": return "float";
                case "bool": return "bool";
                case "string": return "string";
                case "color": return "YiniColor";
                case "coord": return "YiniCoord";
                default: return "object";
            }
        }
    }
}
