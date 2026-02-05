namespace Yini.Model
{
    public class YiniDyna : YiniValue
    {
        public string Expression { get; set; }

        public YiniDyna(string expression)
        {
            Expression = expression;
        }

        public override object GetRawValue() => Expression;

        public override YiniValue Clone()
        {
            var c = new YiniDyna(Expression);
            CopySpanTo(c);
            return c;
        }

        public override string ToString() => $"Dyna({Expression})";
    }
}
