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
            sb.AppendLine("using Yini.Model;"); // Assuming runtime library access
            sb.AppendLine();
            sb.AppendLine($"namespace {namespaceName}");
            sb.AppendLine("{");
            sb.AppendLine($"    public class {className}");
            sb.AppendLine("    {");

            foreach(var sectionKv in doc.Schemas) // Base class on Schemas
            {
                GenerateSectionClass(sb, sectionKv.Key, sectionKv.Value);
            }

            // Also generate properties for accessing sections
            foreach(var sectionKv in doc.Schemas)
            {
                sb.AppendLine($"        public {sectionKv.Key}Config {sectionKv.Key} {{ get; set; }} = new {sectionKv.Key}Config();");
            }

            sb.AppendLine("    }");
            sb.AppendLine("}");
            return sb.ToString();
        }

        private void GenerateSectionClass(StringBuilder sb, string name, YiniSection schema)
        {
            sb.AppendLine($"    public class {name}Config");
            sb.AppendLine("    {");

            foreach(var prop in schema.Properties)
            {
                if (prop.Value is YiniSchemaDefinition def)
                {
                    string type = MapToCSharpType(def.TypeName);
                    sb.AppendLine($"        public {type} {prop.Key} {{ get; set; }}");
                }
            }

            sb.AppendLine("    }");
            sb.AppendLine();
        }

        private string MapToCSharpType(string yiniType)
        {
            switch(yiniType)
            {
                case "int": return "int";
                case "float": return "float";
                case "bool": return "bool";
                case "string": return "string";
                case "color": return "YiniColor"; // Or Engine type? Using Yini model for now
                case "coord": return "YiniCoord";
                default: return "object";
            }
        }
    }
}
