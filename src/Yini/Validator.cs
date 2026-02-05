using System;
using System.Collections.Generic;
using Yini.Model;

namespace Yini
{
    public class ValidationException : Exception
    {
        public ValidationException(string message) : base(message) { }
    }

    public class Validator
    {
        public void Validate(YiniDocument doc)
        {
            foreach (var schemaSectionPair in doc.Schemas)
            {
                var sectionName = schemaSectionPair.Key;
                var schemaSection = schemaSectionPair.Value;

                if (!doc.Sections.ContainsKey(sectionName))
                {
                    // If schema exists for a section, does the section HAVE to exist?
                    // If any property in schema is Required (!), then Yes.
                    // If all are optional, maybe not?
                    // Let's check if any required props.
                    foreach(var prop in schemaSection.Properties)
                    {
                        if (prop.Value is YiniSchemaDefinition def && def.Requirement == SchemaRequirement.Required)
                        {
                            throw new ValidationException($"Missing required section: [{sectionName}]");
                        }
                    }
                    continue; // Section missing but no required props
                }

                var dataSection = doc.Sections[sectionName];
                ValidateSection(dataSection, schemaSection);
            }
        }

        private void ValidateSection(YiniSection data, YiniSection schema)
        {
            foreach (var kv in schema.Properties)
            {
                var key = kv.Key;
                if (!(kv.Value is YiniSchemaDefinition def)) continue;

                bool exists = data.Properties.ContainsKey(key);

                if (!exists)
                {
                    if (def.Requirement == SchemaRequirement.Required)
                    {
                         // If required, but we have a default value strategy?
                         if (def.EmptyBehavior == SchemaEmptyBehavior.Default && def.DefaultValue != null)
                         {
                             data.Properties[key] = def.DefaultValue.Clone();
                         }
                         else
                         {
                             throw new ValidationException($"Missing required property: {key} in [{data.Name}]");
                         }
                    }
                    else // Optional
                    {
                        if (def.EmptyBehavior == SchemaEmptyBehavior.Default && def.DefaultValue != null)
                        {
                            data.Properties[key] = def.DefaultValue.Clone();
                        }
                        else if (def.EmptyBehavior == SchemaEmptyBehavior.Error)
                        {
                             throw new ValidationException($"Property {key} in [{data.Name}] is missing (Error on Empty)");
                        }
                        // Ignore (~) is default
                    }
                }
                else
                {
                    var value = data.Properties[key];
                    ValidateType(key, value, def);
                    ValidateRange(key, value, def);
                }
            }
        }

        private void ValidateType(string key, YiniValue value, YiniSchemaDefinition def)
        {
            if (string.IsNullOrEmpty(def.TypeName)) return;

            string expected = def.TypeName;
            bool valid = false;

            switch (expected)
            {
                case "int": valid = value is YiniInteger; break;
                case "float": valid = value is YiniFloat || value is YiniInteger; break; // Allow int as float
                case "bool": valid = value is YiniBoolean; break;
                case "string": valid = value is YiniString; break;
                case "color": valid = value is YiniColor; break;
                case "coord": valid = value is YiniCoord; break;
                case "path": valid = value is YiniPath; break;
                case "array": valid = value is YiniArray; break;
                case "list": valid = value is YiniList; break;
                case "map": valid = value is YiniMap; break;
                case "struct": valid = value is YiniStruct; break;
                case "dyna": valid = value is YiniDyna; break;
                default:
                    // Complex types like array[int]
                    if (expected.StartsWith("array[") && expected.EndsWith("]"))
                    {
                        if (!(value is YiniArray arr))
                        {
                            valid = false;
                        }
                        else
                        {
                            valid = true;
                            string innerType = expected.Substring(6, expected.Length - 7);
                            foreach(var item in arr.Items)
                            {
                                // We need to check inner type recursively.
                                // But ValidateType takes SchemaDefinition.
                                // We can create a dummy definition.
                                var dummyDef = new YiniSchemaDefinition { TypeName = innerType };
                                ValidateType(key + "[]", item, dummyDef);
                            }
                        }
                    }
                    else
                    {
                        // Unknown type, assume valid or warn?
                        valid = true;
                    }
                    break;
            }

            if (!valid)
            {
                throw new ValidationException($"Property {key} expected type {expected}, got {value.GetType().Name}");
            }
        }

        private void ValidateRange(string key, YiniValue value, YiniSchemaDefinition def)
        {
            if (def.Min == null && def.Max == null) return;

            float val = 0;
            bool isNum = false;

            if (value is YiniInteger i) { val = i.Value; isNum = true; }
            else if (value is YiniFloat f) { val = f.Value; isNum = true; }

            if (!isNum) return; // Range only for numbers

            if (def.Min != null)
            {
                float min = (def.Min is YiniInteger mi) ? mi.Value : ((YiniFloat)def.Min).Value;
                if (val < min) throw new ValidationException($"Property {key} value {val} is less than min {min}");
            }

            if (def.Max != null)
            {
                 float max = (def.Max is YiniInteger ma) ? ma.Value : ((YiniFloat)def.Max).Value;
                 if (val > max) throw new ValidationException($"Property {key} value {val} is greater than max {max}");
            }
        }
    }
}
