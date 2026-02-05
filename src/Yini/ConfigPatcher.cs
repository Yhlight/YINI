using System;
using System.Collections.Generic;
using Yini.Model;

namespace Yini
{
    public static class ConfigPatcher
    {
        public static void ApplyPatch(YiniDocument baseDoc, YiniDocument patchDoc)
        {
            foreach (var sectionKv in patchDoc.Sections)
            {
                var sectionName = sectionKv.Key;
                var patchSection = sectionKv.Value;

                if (!baseDoc.Sections.ContainsKey(sectionName))
                {
                    // New section, add it
                    // Deep clone to be safe
                    // For now, assume patchDoc ownership is fine or we clone
                    baseDoc.Sections[sectionName] = patchSection;
                }
                else
                {
                    var baseSection = baseDoc.Sections[sectionName];

                    // Merge Properties
                    foreach (var prop in patchSection.Properties)
                    {
                        baseSection.Properties[prop.Key] = prop.Value; // Overwrite
                    }

                    // Append Registry?
                    // Usually "Patch" implies modifying existing or adding new.
                    // Appending registry is safer than overwriting.
                    baseSection.Registry.AddRange(patchSection.Registry);
                }
            }
        }
    }
}
