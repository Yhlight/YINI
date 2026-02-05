using System;
using System.Collections.Generic;
using UnityEngine;
using Yini;
using Yini.Model;

namespace Yini.Unity
{
    public class YiniManager : MonoBehaviour, IEvaluationContext
    {
        public static YiniManager Instance { get; private set; }

        public YiniAsset DefaultConfig;

        private YiniDocument _doc;
        public YiniDocument Document => _doc;

        private Evaluator _evaluator;
        private Dictionary<string, YiniValue> _runtimeVariables = new Dictionary<string, YiniValue>();

        private void Awake()
        {
            if (Instance == null)
            {
                Instance = this;
                DontDestroyOnLoad(gameObject);
            }
            else
            {
                Destroy(gameObject);
                return;
            }

            _evaluator = new Evaluator(this);
            if (DefaultConfig != null)
            {
                LoadConfig(DefaultConfig.Source);
            }
        }

        public void LoadConfig(string source)
        {
            var compiler = new Compiler();
            _doc = compiler.Compile(source);
        }

        public void ApplyPatch(string patchContent)
        {
            if (_doc == null) return;
            var compiler = new Compiler();
            var patchDoc = compiler.Compile(patchContent);
            ConfigPatcher.ApplyPatch(_doc, patchDoc);
        }

        public T Get<T>(string section, string key, T defaultValue = default)
        {
            if (_doc == null || !_doc.Sections.ContainsKey(section)) return defaultValue;
            var sec = _doc.Sections[section];
            if (!sec.Properties.ContainsKey(key)) return defaultValue;

            var val = sec.Properties[key];

            // If Dyna, evaluate it
            if (val is YiniDyna dyna)
            {
                val = _evaluator.EvaluateDyna(dyna.Expression);
            }

            // Convert val to T
            return ConvertValue<T>(val, defaultValue);
        }

        public void SetVariable(string name, YiniValue value)
        {
            _runtimeVariables[name] = value;
        }

        // IEvaluationContext Implementation
        public YiniValue ResolveReference(YiniReference reference)
        {
            // Support @{Section.Key}
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
            if (_runtimeVariables.ContainsKey(name)) return _runtimeVariables[name];

            // Built-ins
            if (name == "Time") return new YiniFloat(Time.time);
            if (name == "DeltaTime") return new YiniFloat(Time.deltaTime);

            return null;
        }

        private T ConvertValue<T>(YiniValue val, T def)
        {
            try
            {
                if (typeof(T) == typeof(int) && val is YiniInteger i) return (T)(object)i.Value;
                if (typeof(T) == typeof(float))
                {
                    if (val is YiniFloat f) return (T)(object)f.Value;
                    if (val is YiniInteger i2) return (T)(object)(float)i2.Value;
                }
                if (typeof(T) == typeof(string) && val is YiniString s) return (T)(object)s.Value;
                if (typeof(T) == typeof(bool) && val is YiniBoolean b) return (T)(object)b.Value;
                if (typeof(T) == typeof(Color) && val is YiniColor c)
                    return (T)(object)new Color(c.R/255f, c.G/255f, c.B/255f, c.A/255f);
                if (typeof(T) == typeof(Vector2) && val is YiniCoord v2) return (T)(object)new Vector2(v2.X, v2.Y);
                if (typeof(T) == typeof(Vector3) && val is YiniCoord v3) return (T)(object)new Vector3(v3.X, v3.Y, v3.Z);
            }
            catch {}
            return def;
        }
    }
}
