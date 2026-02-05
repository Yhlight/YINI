using System;
using System.Collections.Generic;
using System.Linq;
using Yini.Model;

namespace Yini
{
    public class Compiler
    {
        private YiniDocument _doc;
        private Dictionary<string, YiniValue> _macros;

        private HashSet<string> _resolvedSections;
        private HashSet<string> _resolvingSections;

        public YiniDocument Compile(string source)
        {
            _macros = new Dictionary<string, YiniValue>();
            _resolvedSections = new HashSet<string>();
            _resolvingSections = new HashSet<string>();

            var lexer = new Lexer(source);
            var tokens = lexer.Tokenize();
            var parser = new Parser(tokens);
            _doc = parser.Parse();

            // 2. Gather Macros
            if (_doc.Sections.ContainsKey("#define"))
            {
                var defineSection = _doc.Sections["#define"];
                foreach(var kv in defineSection.Properties)
                {
                    _macros[kv.Key] = kv.Value;
                }
                // Also from doc.Macros if populated
                foreach(var kv in _doc.Macros) _macros[kv.Key] = kv.Value;

                _doc.Macros = _macros;
            }

            // 3. Resolve Sections
            foreach(var sectionName in new List<string>(_doc.Sections.Keys))
            {
                if (sectionName.StartsWith("#")) continue;
                ResolveSection(sectionName);
            }

            return _doc;
        }

        private void ResolveSection(string name)
        {
            if (_resolvedSections.Contains(name)) return;
            if (_resolvingSections.Contains(name)) throw new Exception($"Circular inheritance detected: {name}");

            _resolvingSections.Add(name);
            var section = _doc.Sections[name];

            // Resolve Parents
            foreach(var parentName in section.Parents)
            {
                if (!_doc.Sections.ContainsKey(parentName))
                {
                    throw new Exception($"Parent section {parentName} not found");
                }
                ResolveSection(parentName);

                var parent = _doc.Sections[parentName];

                // Merge Properties (Child overrides)
                foreach(var kv in parent.Properties)
                {
                    if (!section.Properties.ContainsKey(kv.Key))
                    {
                        section.Properties[kv.Key] = kv.Value.Clone();
                    }
                }
            }

            // Resolve Values
            var keys = new List<string>(section.Properties.Keys);
            foreach(var key in keys)
            {
                section.Properties[key] = ResolveValue(section.Properties[key], section);
            }

            // Resolve Registry
            for(int i=0; i<section.Registry.Count; i++)
            {
                section.Registry[i] = ResolveValue(section.Registry[i], section);
            }

            _resolvingSections.Remove(name);
            _resolvedSections.Add(name);
        }

        private YiniValue ResolveValue(YiniValue value, YiniSection context)
        {
            if (value is YiniReference reference)
            {
                return ResolveReference(reference, context);
            }
            if (value is YiniBinaryExpression binary)
            {
                return EvaluateBinary(binary, context);
            }
            if (value is YiniUnaryExpression unary)
            {
                return EvaluateUnary(unary, context);
            }

            if (value is YiniArray arr)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in arr.Items) newItems.Add(ResolveValue(item, context));
                return new YiniArray(newItems);
            }
            if (value is YiniList list)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in list.Items) newItems.Add(ResolveValue(item, context));
                return new YiniList(newItems);
            }
            if (value is YiniSet set)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in set.Elements) newItems.Add(ResolveValue(item, context));
                return new YiniSet(newItems);
            }
            if (value is YiniMap map)
            {
                 // Resolve values in map
                 var keys = new List<string>(map.Items.Keys);
                 foreach(var k in keys)
                 {
                     map.Items[k] = ResolveValue(map.Items[k], context);
                 }
                 return map;
            }

            return value;
        }

        private YiniValue ResolveReference(YiniReference refVal, YiniSection context)
        {
            if (refVal.Type == ReferenceType.Macro)
            {
                if (_macros.ContainsKey(refVal.Reference))
                {
                    return ResolveValue(_macros[refVal.Reference], context);
                }
                throw new Exception($"Undefined macro: @{refVal.Reference}");
            }
            if (refVal.Type == ReferenceType.Environment)
            {
                var val = Environment.GetEnvironmentVariable(refVal.Reference);
                if (val == null) return new YiniString(""); // Or throw
                return new YiniString(val);
            }
            if (refVal.Type == ReferenceType.CrossSection)
            {
                var parts = refVal.Reference.Split('.');
                if (parts.Length < 2) throw new Exception("Invalid section reference format");

                string secName = parts[0];
                string key = parts[1];

                if (_doc.Sections.ContainsKey(secName))
                {
                    ResolveSection(secName);
                    var sec = _doc.Sections[secName];
                    if (sec.Properties.ContainsKey(key))
                    {
                        return sec.Properties[key];
                    }
                }
                throw new Exception($"Reference not found: {refVal.Reference}");
            }
            return refVal;
        }

        private YiniValue EvaluateBinary(YiniBinaryExpression expr, YiniSection context)
        {
            var left = ResolveValue(expr.Left, context);
            var right = ResolveValue(expr.Right, context);

            if (left is YiniInteger lInt && right is YiniInteger rInt)
            {
                switch(expr.Op)
                {
                    case TokenType.Plus: return new YiniInteger(lInt.Value + rInt.Value);
                    case TokenType.Minus: return new YiniInteger(lInt.Value - rInt.Value);
                    case TokenType.Multiply: return new YiniInteger(lInt.Value * rInt.Value);
                    case TokenType.Divide: return new YiniInteger(lInt.Value / rInt.Value);
                    case TokenType.Modulo: return new YiniInteger(lInt.Value % rInt.Value);
                }
            }
            if ((left is YiniFloat || left is YiniInteger) && (right is YiniFloat || right is YiniInteger))
            {
                float lVal = (left is YiniFloat lf) ? lf.Value : ((YiniInteger)left).Value;
                float rVal = (right is YiniFloat rf) ? rf.Value : ((YiniInteger)right).Value;
                 switch(expr.Op)
                {
                    case TokenType.Plus: return new YiniFloat(lVal + rVal);
                    case TokenType.Minus: return new YiniFloat(lVal - rVal);
                    case TokenType.Multiply: return new YiniFloat(lVal * rVal);
                    case TokenType.Divide: return new YiniFloat(lVal / rVal);
                    // Modulo on float?
                    case TokenType.Modulo: return new YiniFloat(lVal % rVal);
                }
            }

            throw new Exception($"Cannot apply operator {expr.Op} to {left} and {right}");
        }

        private YiniValue EvaluateUnary(YiniUnaryExpression expr, YiniSection context)
        {
            var operand = ResolveValue(expr.Operand, context);
             if (operand is YiniInteger i)
            {
                if (expr.Op == TokenType.Minus) return new YiniInteger(-i.Value);
            }
            if (operand is YiniFloat f)
            {
                if (expr.Op == TokenType.Minus) return new YiniFloat(-f.Value);
            }
            throw new Exception($"Cannot apply unary operator {expr.Op} to {operand}");
        }
    }
}
