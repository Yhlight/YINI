namespace Yini.Model
{
    public enum SchemaRequirement
    {
        Required, // !
        Optional  // ?
    }

    public enum SchemaEmptyBehavior
    {
        Ignore, // ~
        Error,  // e
        Default // = (Value provided)
    }

    public class YiniSchemaDefinition : YiniValue
    {
        public SchemaRequirement Requirement { get; set; } = SchemaRequirement.Optional;
        public string TypeName { get; set; }
        public YiniValue DefaultValue { get; set; }
        public YiniValue Min { get; set; }
        public YiniValue Max { get; set; }
        public SchemaEmptyBehavior EmptyBehavior { get; set; } = SchemaEmptyBehavior.Ignore;

        public override object GetRawValue() => this;
        public override YiniValue Clone()
        {
            var c = new YiniSchemaDefinition
            {
                Requirement = Requirement,
                TypeName = TypeName,
                DefaultValue = DefaultValue?.Clone(),
                Min = Min?.Clone(),
                Max = Max?.Clone(),
                EmptyBehavior = EmptyBehavior
            };
            CopySpanTo(c);
            return c;
        }

        public override string ToString() => $"Schema({TypeName}, {Requirement})";
    }
}
