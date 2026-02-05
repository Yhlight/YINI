using Godot;
using System.Collections.Generic;
using Yini;
using Yini.Model;

namespace Yini.Godot
{
    public partial class YiniRuntime : Node, IEvaluationContext
    {
        public static YiniRuntime Instance { get; private set; }

        private YiniDocument _doc;
        private Evaluator _evaluator;
        private Dictionary<string, YiniValue> _variables = new Dictionary<string, YiniValue>();

        public override void _EnterTree()
        {
            Instance = this;
            _evaluator = new Evaluator(this);
        }

        public void LoadConfig(string path)
        {
             using var file = FileAccess.Open(path, FileAccess.ModeFlags.Read);
             if (file == null)
             {
                 GD.PrintErr($"Failed to open {path}");
                 return;
             }
             string content = file.GetAsText();
             var compiler = new Compiler();
             _doc = compiler.Compile(content);
        }

        public void ApplyPatch(string patchContent)
        {
            if (_doc == null) return;
            var compiler = new Compiler();
            var patchDoc = compiler.Compile(patchContent);
            ConfigPatcher.ApplyPatch(_doc, patchDoc);
        }

        public Variant Get(string section, string key, Variant defaultValue = default)
        {
             if (_doc == null || !_doc.Sections.ContainsKey(section)) return defaultValue;
             var sec = _doc.Sections[section];
             if (!sec.Properties.ContainsKey(key)) return defaultValue;

             var val = sec.Properties[key];
             if (val is YiniDyna dyna)
             {
                 val = _evaluator.EvaluateDyna(dyna.Expression);
             }

             return ConvertValue(val);
        }

        public YiniValue ResolveReference(YiniReference reference)
        {
             if (reference.Type == ReferenceType.CrossSection)
            {
                var parts = reference.Reference.Split('.');
                if (parts.Length >= 2 && _doc.Sections.ContainsKey(parts[0]))
                {
                    var sec = _doc.Sections[parts[0]];
                    if (sec.Properties.ContainsKey(parts[1]))
                        return sec.Properties[parts[1]];
                }
            }
            return reference;
        }

        public YiniValue ResolveVariable(string name)
        {
            if (_variables.ContainsKey(name)) return _variables[name];

            if (name == "Time") return new YiniFloat((float)Time.GetTicksMsec() / 1000f);
            return null;
        }

        private Variant ConvertValue(YiniValue val)
        {
             if (val is YiniInteger i) return i.Value;
             if (val is YiniFloat f) return f.Value;
             if (val is YiniBoolean b) return b.Value;
             if (val is YiniString s) return s.Value;
             if (val is YiniColor c) return new Color(c.R/255f, c.G/255f, c.B/255f, c.A/255f);
             if (val is YiniCoord co) return co.Is3D ? new Vector3(co.X, co.Y, co.Z) : new Vector2(co.X, co.Y);
             if (val is YiniArray arr)
             {
                 var a = new global::Godot.Collections.Array();
                 foreach(var item in arr.Items) a.Add(ConvertValue(item));
                 return a;
             }
             return val.ToString();
        }
    }
}
