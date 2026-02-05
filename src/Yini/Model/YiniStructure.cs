using System.Collections.Generic;

namespace Yini.Model
{
    public class YiniSection
    {
        public string Name { get; set; }
        public List<string> Parents { get; } = new List<string>();
        public Dictionary<string, YiniValue> Properties { get; } = new Dictionary<string, YiniValue>();
        // Quick registration list (+=)
        public List<YiniValue> Registry { get; } = new List<YiniValue>();

        public YiniSection(string name)
        {
            Name = name;
        }

        public override string ToString() => $"[{Name}]";
    }

    public class YiniDocument
    {
        public Dictionary<string, YiniSection> Sections { get; } = new Dictionary<string, YiniSection>();

        public Dictionary<string, YiniValue> Macros { get; set; } = new Dictionary<string, YiniValue>();

        // Schemas can be stored separately or as sections with special handling.
        // For now, let's keep them separate to distinguish configuration from validation rules.
        public Dictionary<string, YiniSection> Schemas { get; } = new Dictionary<string, YiniSection>();
    }
}
