using System;

namespace Yini.Model
{
    public class YiniBinaryExpression : YiniValue
    {
        public YiniValue Left { get; }
        public YiniValue Right { get; }
        public TokenType Op { get; }

        public YiniBinaryExpression(YiniValue left, TokenType op, YiniValue right)
        {
            Left = left;
            Op = op;
            Right = right;
        }

        public override object GetRawValue() => throw new InvalidOperationException($"Unresolved Binary Expression: {Left} {Op} {Right}");
        public override YiniValue Clone() { var c = new YiniBinaryExpression(Left.Clone(), Op, Right.Clone()); CopySpanTo(c); return c; }
        public override string ToString() => $"({Left} {Op} {Right})";
    }

    public class YiniUnaryExpression : YiniValue
    {
        public YiniValue Operand { get; }
        public TokenType Op { get; }

        public YiniUnaryExpression(TokenType op, YiniValue operand)
        {
            Op = op;
            Operand = operand;
        }

        public override object GetRawValue() => throw new InvalidOperationException($"Unresolved Unary Expression: {Op}{Operand}");
        public override YiniValue Clone() { var c = new YiniUnaryExpression(Op, Operand.Clone()); CopySpanTo(c); return c; }
        public override string ToString() => $"({Op}{Operand})";
    }
}
