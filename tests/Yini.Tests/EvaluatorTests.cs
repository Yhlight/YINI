using System.Collections.Generic;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class MockEvaluationContext : IEvaluationContext
    {
        public Dictionary<string, YiniValue> Variables { get; } = new Dictionary<string, YiniValue>();

        public YiniValue ResolveReference(YiniReference reference)
        {
            if (reference.Type == ReferenceType.Macro && Variables.ContainsKey(reference.Reference))
                return Variables[reference.Reference];
            return reference;
        }

        public YiniValue ResolveVariable(string name)
        {
            if (Variables.ContainsKey(name)) return Variables[name];
            return null;
        }
    }

    public class EvaluatorTests
    {
        [Fact]
        public void TestEvaluateSimpleMath()
        {
            var ctx = new MockEvaluationContext();
            var evaluator = new Evaluator(ctx);

            var result = evaluator.EvaluateDyna("1 + 2 * 3");
            Assert.IsType<YiniInteger>(result);
            Assert.Equal(7, ((YiniInteger)result).Value);
        }

        [Fact]
        public void TestEvaluateWithVariables()
        {
            var ctx = new MockEvaluationContext();
            ctx.Variables["Time"] = new YiniFloat(1.5f);
            ctx.Variables["Speed"] = new YiniInteger(10);

            var evaluator = new Evaluator(ctx);

            var result = evaluator.EvaluateDyna("Time * Speed");
            Assert.IsType<YiniFloat>(result);
            Assert.Equal(15.0f, ((YiniFloat)result).Value);
        }

        [Fact]
        public void TestEvaluateDynaString()
        {
            var ctx = new MockEvaluationContext();
            var evaluator = new Evaluator(ctx);

            // "Hello " + "World"
            var result = evaluator.EvaluateDyna("\"Hello \" + \"World\"");
            Assert.IsType<YiniString>(result);
            Assert.Equal("Hello World", ((YiniString)result).Value);
        }
    }
}
