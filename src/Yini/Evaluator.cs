using System;
using System.Collections.Generic;
using Yini.Bytecode;
using Yini.Model;

namespace Yini
{
    public interface IEvaluationContext
    {
        YiniValue ResolveReference(YiniReference reference);
        YiniValue ResolveVariable(string name); // For Dyna
    }

    public class Evaluator
    {
        private readonly IEvaluationContext _context;
        private readonly Dictionary<string, byte[]> _bytecodeCache = new Dictionary<string, byte[]>();
        private readonly BytecodeVM _vm;

        public Evaluator(IEvaluationContext context)
        {
            _context = context;
            _vm = new BytecodeVM(context);
        }

        public YiniValue EvaluateDyna(string expression, bool useBytecode = false)
        {
            if (useBytecode)
            {
                if (!_bytecodeCache.TryGetValue(expression, out byte[] code))
                {
                    var compiler = new BytecodeCompiler();
                    var lexer = new Lexer(expression);
                    var parser = new Parser(lexer.Tokenize());
                    var ast = parser.ParseExpression();
                    code = compiler.Compile(ast);
                    _bytecodeCache[expression] = code;
                }
                return _vm.Run(code);
            }
            else
            {
                var lexer = new Lexer(expression);
                var tokens = lexer.Tokenize();
                var parser = new Parser(tokens);
                var ast = parser.ParseExpression();
                return ResolveValue(ast, allowVariables: true);
            }
        }

        public YiniValue ResolveValue(YiniValue value, bool allowVariables = false)
        {
            if (allowVariables && value is YiniString str)
            {
                var varVal = _context.ResolveVariable(str.Value);
                if (varVal != null) return ResolveValue(varVal, allowVariables);
                // If not found, keep as string
            }

            if (value is YiniReference reference)
            {
                var resolved = _context.ResolveReference(reference);
                return ResolveValue(resolved, allowVariables);
            }
            if (value is YiniBinaryExpression binary)
            {
                return EvaluateBinary(binary, allowVariables);
            }
            if (value is YiniUnaryExpression unary)
            {
                return EvaluateUnary(unary, allowVariables);
            }

            if (value is YiniDyna) return value;

            if (value is YiniArray arr)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in arr.Items) newItems.Add(ResolveValue(item, allowVariables));
                return new YiniArray(newItems);
            }
            if (value is YiniList list)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in list.Items) newItems.Add(ResolveValue(item, allowVariables));
                return new YiniList(newItems);
            }
            if (value is YiniSet set)
            {
                var newItems = new List<YiniValue>();
                foreach(var item in set.Elements) newItems.Add(ResolveValue(item, allowVariables));
                return new YiniSet(newItems);
            }
            if (value is YiniMap map)
            {
                 var keys = new List<string>(map.Items.Keys);
                 foreach(var k in keys)
                 {
                     map.Items[k] = ResolveValue(map.Items[k], allowVariables);
                 }
                 return map;
            }
            if (value is YiniStruct structVal)
            {
                var keys = new List<string>(structVal.Fields.Keys);
                foreach(var k in keys)
                {
                    structVal.Fields[k] = ResolveValue(structVal.Fields[k], allowVariables);
                }
                return structVal;
            }

            return value;
        }

        private YiniValue EvaluateBinary(YiniBinaryExpression expr, bool allowVariables)
        {
            var left = ResolveValue(expr.Left, allowVariables);
            var right = ResolveValue(expr.Right, allowVariables);

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
                    case TokenType.Modulo: return new YiniFloat(lVal % rVal);
                }
            }

            // Allow string concatenation if one is string?
            if (expr.Op == TokenType.Plus && (left is YiniString || right is YiniString))
            {
                return new YiniString(GetValueString(left) + GetValueString(right));
            }

            throw new Exception($"Cannot apply operator {expr.Op} to {left} and {right}");
        }

        private string GetValueString(YiniValue val)
        {
            if (val is YiniString s) return s.Value;
            if (val is YiniInteger i) return i.Value.ToString();
            if (val is YiniFloat f) return f.Value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            if (val is YiniBoolean b) return b.Value.ToString().ToLower();
            return val.ToString();
        }

        private YiniValue EvaluateUnary(YiniUnaryExpression expr, bool allowVariables)
        {
            var operand = ResolveValue(expr.Operand, allowVariables);
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
