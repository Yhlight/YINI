using System;
using System.Collections.Generic;
using System.IO;
using Yini.Model;

namespace Yini.Bytecode
{
    public enum OpCode : byte
    {
        Halt = 0,
        PushInt = 1,
        PushFloat = 2,
        PushString = 3,
        PushBool = 4,
        PushVar = 5,

        Add = 10,
        Sub = 11,
        Mul = 12,
        Div = 13,
        Mod = 14,
        Neg = 15
    }

    public class BytecodeCompiler
    {
        private List<byte> _code = new List<byte>();
        private List<object> _constants = new List<object>();

        public byte[] Compile(YiniValue ast)
        {
            _code.Clear();
            _constants.Clear();
            EmitNode(ast);
            Emit(OpCode.Halt);

            // Format: [CountConsts (4b)] [Consts...] [Code...]
            // For simplicity, let's just return code and assume self-contained or use a container class.
            // Let's make a BytecodeProgram container?
            // For now, let's just serialize everything into the byte array.

            using (var ms = new MemoryStream())
            using (var writer = new BinaryWriter(ms))
            {
                writer.Write(_constants.Count);
                foreach(var c in _constants)
                {
                    if (c is int i) { writer.Write((byte)1); writer.Write(i); }
                    else if (c is float f) { writer.Write((byte)2); writer.Write(f); }
                    else if (c is string s) { writer.Write((byte)3); writer.Write(s); }
                    else if (c is bool b) { writer.Write((byte)4); writer.Write(b); }
                }
                writer.Write(_code.ToArray());
                return ms.ToArray();
            }
        }

        private void Emit(OpCode op) => _code.Add((byte)op);

        private void EmitConst(OpCode op, object val)
        {
            Emit(op);
            _constants.Add(val);
            int idx = _constants.Count - 1;
            // Write index as 2 bytes?
            _code.Add((byte)(idx & 0xFF));
            _code.Add((byte)((idx >> 8) & 0xFF));
        }

        private void EmitNode(YiniValue node)
        {
            if (node is YiniInteger i) EmitConst(OpCode.PushInt, i.Value);
            else if (node is YiniFloat f) EmitConst(OpCode.PushFloat, f.Value);
            else if (node is YiniBoolean b) EmitConst(OpCode.PushBool, b.Value);
            else if (node is YiniString s)
            {
                // Is it a var? Eval treats string as var lookup if allowed.
                // But compiler doesn't know context.
                // Assuming Strings are just strings, identifiers (if we had them in AST separate from string) are vars.
                // In Parser, identifiers become YiniString (unquoted) or YiniString (quoted).
                // Evaluator logic: `if (allowVariables && value is YiniString str)`
                // So we need to emit PushVar if it looks like a var?
                // Or let the VM handle it?
                // Let's assume all strings are potentially vars if they look like identifiers?
                // Or just emit PushString, and VM tries to resolve?
                // OpCode.PushString vs OpCode.PushVar.
                // Let's emit PushVar for now if it's a "symbol".
                // But YiniString doesn't distinguish.
                // We'll emit PushString. VM instruction `Resolve`?
                // Or just `PushVar` which takes a string constant index.
                EmitConst(OpCode.PushVar, s.Value);
            }
            else if (node is YiniBinaryExpression bin)
            {
                EmitNode(bin.Left);
                EmitNode(bin.Right);
                switch(bin.Op)
                {
                    case TokenType.Plus: Emit(OpCode.Add); break;
                    case TokenType.Minus: Emit(OpCode.Sub); break;
                    case TokenType.Multiply: Emit(OpCode.Mul); break;
                    case TokenType.Divide: Emit(OpCode.Div); break;
                    case TokenType.Modulo: Emit(OpCode.Mod); break;
                }
            }
            else if (node is YiniUnaryExpression un)
            {
                EmitNode(un.Operand);
                if (un.Op == TokenType.Minus) Emit(OpCode.Neg);
            }
        }
    }

    public class BytecodeVM
    {
        private IEvaluationContext _ctx;

        public BytecodeVM(IEvaluationContext ctx)
        {
            _ctx = ctx;
        }

        public YiniValue Run(byte[] program)
        {
            using (var ms = new MemoryStream(program))
            using (var reader = new BinaryReader(ms))
            {
                int constCount = reader.ReadInt32();
                var consts = new object[constCount];
                for(int k=0; k<constCount; k++)
                {
                    byte type = reader.ReadByte();
                    if (type == 1) consts[k] = reader.ReadInt32();
                    else if (type == 2) consts[k] = reader.ReadSingle();
                    else if (type == 3) consts[k] = reader.ReadString();
                    else if (type == 4) consts[k] = reader.ReadBoolean();
                }

                // Code starts here
                var stack = new Stack<YiniValue>();

                while (ms.Position < ms.Length)
                {
                    OpCode op = (OpCode)reader.ReadByte();
                    if (op == OpCode.Halt) break;

                    switch(op)
                    {
                        case OpCode.PushInt:
                        {
                            int idx = reader.ReadByte() | (reader.ReadByte() << 8);
                            stack.Push(new YiniInteger((int)consts[idx]));
                            break;
                        }
                        case OpCode.PushFloat:
                        {
                            int idx = reader.ReadByte() | (reader.ReadByte() << 8);
                            stack.Push(new YiniFloat((float)consts[idx]));
                            break;
                        }
                        case OpCode.PushVar:
                        {
                            int idx = reader.ReadByte() | (reader.ReadByte() << 8);
                            string name = (string)consts[idx];
                            // Try resolve
                            var val = _ctx.ResolveVariable(name);
                            if (val != null) stack.Push(val);
                            else stack.Push(new YiniString(name)); // Fallback literal
                            break;
                        }
                        case OpCode.Add:
                        {
                            var r = stack.Pop();
                            var l = stack.Pop();
                            stack.Push(Add(l, r));
                            break;
                        }
                        case OpCode.Mul:
                        {
                            var r = stack.Pop();
                            var l = stack.Pop();
                            stack.Push(Mul(l, r));
                            break;
                        }
                        // Implement others...
                    }
                }

                return stack.Count > 0 ? stack.Pop() : null;
            }
        }

        private YiniValue Add(YiniValue l, YiniValue r)
        {
             if (l is YiniInteger li && r is YiniInteger ri) return new YiniInteger(li.Value + ri.Value);
             if (l is YiniFloat lf || r is YiniFloat rf) return new YiniFloat(GetFloat(l) + GetFloat(r));
             return l;
        }

        private YiniValue Mul(YiniValue l, YiniValue r)
        {
             if (l is YiniInteger li && r is YiniInteger ri) return new YiniInteger(li.Value * ri.Value);
             return new YiniFloat(GetFloat(l) * GetFloat(r));
        }

        private float GetFloat(YiniValue v)
        {
            if (v is YiniFloat f) return f.Value;
            if (v is YiniInteger i) return i.Value;
            return 0f;
        }
    }
}
