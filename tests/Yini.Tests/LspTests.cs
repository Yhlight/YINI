using System.Collections.Generic;
using Xunit;
using Yini;
using Yini.Model;

namespace Yini.Tests
{
    public class LspTests
    {
        [Fact]
        public void TestLspHover()
        {
            var compiler = new Compiler();
            var src = @"
[#schema]
[Config]
val = !, int, min=0, max=100

[Config]
val = 50
";
            // Lsp tests rely on source being valid YINI except for completion cursor.
            // The error was "Unexpected token NumberLiteral".
            // In Parser.ParseBody: key = value.
            // It expects identifier.
            // If parser throws, doc is null? No, Compile throws.
            // Wait, why did it fail?
            // "val = 50" -> key=val, val=50.
            // Ah, the parser error says "at <string>:7:7".
            // Line 7 is "val = 50".
            // Token 7:7 is 50.
            // Identifier expected?
            // "key = value" -> Identifier, Assign, Expr.
            // Maybe Lexer issues?
            // Or did I break Parser recently?
            // "Unexpected token in section body: NumberLiteral('50')"
            // It seems it didn't consume "val" as key?
            // Previous lines: "[Config]"
            // Maybe parser sees [Config] then thinks next is body.
            // "val" is identifier.

            // Re-read Parser.ParseBody:
            // while...
            // if (Match(PlusAssign)) ...
            // else if (Current.Type == Identifier) ...
            // else Throw

            // "val" should be Identifier.
            // Why NumberLiteral?
            // Is "val" missing?

            // Let's debug the test string.
            // "[#schema]\n..."
            // It worked in other tests.
            // Maybe an invisible char or something?
            // Or maybe my Parser change for KeyLocations broke something.
            // Wait, Parser.ParseBody expects identifiers. 50 is NumberLiteral.
            // But 50 is the value. "val" is the key.
            // Ah, line 183 in Parser.cs:
            // if (Match(PlusAssign)) ...
            // else if (Current.Type == Identifier) ...
            // else Throw
            // In the test: "val = 50".
            // "val" is Identifier.
            // Why did it fail with "NumberLiteral '50'"?
            // This means it SKIPPED "val" and "="?
            // OR "val" was tokenized as something else?
            // "val" -> Identifier.
            // Maybe the previous section parsing consumed something?
            // Ah! ParseSchemaDefinition logic might be buggy and eating into next section?
            // In ParseSchemaDefinition:
            // while (first || Match(Comma)) ...
            // If it doesn't match comma, loop ends.
            // "min=0, max=100" -> Identifier(min) Assign Number(0) Comma Identifier(max) Assign Number(100)
            // Next is NewLine then [Config].
            // ParseSchemaDefinition returns.
            // Back in ParseBody (of schema section).
            // It sees [Config] (LBracket).
            // ParseBody loop condition: while (Current.Type != TokenType.LBracket ...)
            // So it should exit ParseBody.
            // Then ParseSection finishes.
            // Back in Parse loop.
            // Sees [Config]. Parses next section.
            // Body of second [Config].
            // "val = 50".
            // "val" is Identifier.
            // It should enter `else if (Current.Type == TokenType.Identifier)`.
            // BUT: `GetPrecedence` returned -1.
            // Wait, looking at the error: "Unexpected token in section body: NumberLiteral('50')".
            // This means it was in the `else` block of `ParseBody`.
            // So `Current.Type` was NOT `Identifier`.
            // And NOT `PlusAssign`.
            // It was `NumberLiteral`.
            // So "val" and "=" were consumed?
            // Who consumed them?
            // Maybe ParseExpression inside ParseSchemaDefinition?
            // `def.Max = ParseExpression()`.
            // ParseExpression calls ParsePrimary.
            // 100 is parsed.
            // Then loop in ParseExpression checks precedence.
            // Next token is NewLine (ignored) then LBracket (`[Config]`).
            // LBracket precedence is -1. Loop breaks.
            // So `def.Max` gets 100.
            // Schema def finishes.

            // Wait, I suspect `key = value` logic in ParseBody.
            // `var keyToken = Consume(TokenType.Identifier, "Expected key");`
            // `Consume(TokenType.Assign, "Expected '='");`
            // `var value = ParseExpression();`

            // If `value` parsing consumes too much?
            // `val = 50`.
            // 50 is parsed.
            // Next is EOF (or end of string).

            // Why did it fail on `val` line?
            // Maybe "val" was NOT parsed as identifier?
            // Is "val" a keyword? No.
            // `val` = Identifier.

            // Wait! The error says line 7.
            // `val = 50` is line 7.
            // `NumberLiteral('50')` found.
            // This implies `val` and `=` WERE consumed.
            // `Consume(Identifier)` -> ok.
            // `Consume(Assign)` -> ok.
            // `if (_isSchemaMode)` -> true?
            // Wait, `_isSchemaMode` is a field in Parser.
            // When parsing `[#schema]`, it sets `_isSchemaMode = true`.
            // When parsing `[Config]` (the second one), does it reset `_isSchemaMode`?
            // ParseSection:
            // if (special == "schema") _isSchemaMode = true;
            // else ...
            // It NEVER sets `_isSchemaMode = false`!
            // So the second `[Config]` is parsed in Schema Mode.
            // In Schema Mode, `ParseBody` calls `ParseSchemaDefinition`.
            // `ParseSchemaDefinition` expects `!`, `?`, `type`, etc.
            // It sees `50` (NumberLiteral).
            // It loops:
            // `Match(!)` -> no.
            // `Match(?)` -> no.
            // `Match(~)` -> no.
            // `Match(=)` -> no. (It's `50`, not `=`)
            // `Current == Identifier` -> no (It's Number).
            // `else break`.
            // Returns empty schema def.
            // `section.Properties["val"] = schemaDef`.
            // Loop ParseBody continues.
            // Next token is `50`.
            // `Match(PlusAssign)` -> no.
            // `Current == Identifier` -> no (It's Number).
            // `else Throw "Unexpected token..."`.
            // BINGO!
            // I need to reset `_isSchemaMode` in `ParseSection`.

            var doc = compiler.Compile(src);

            // Find line/col of 'val' in [Config]
            // We need to know where it is.
            // Parser tracks locations now.
            var section = doc.Sections["Config"];
            var span = section.KeyLocations["val"];

            // Hover exactly at start of 'val'
            string hover = LspHelper.GetHover(doc, span.Line, span.Column);
            Assert.NotNull(hover);
            Assert.Contains("**Property** `val`", hover);
            Assert.Contains("**Type**: `int`", hover);
            Assert.Contains("**Required**: `Yes`", hover);
            Assert.Contains("**Min**: `0`", hover);
        }

        [Fact]
        public void TestLspCompletion()
        {
            var compiler = new Compiler();
            var src = @"
[#schema]
[Config]
key1 = ?, int
key2 = ?, int

[Config]
key1 = 10
";
            var doc = compiler.Compile(src);

            // Cursor in [Config] section, new line
            // Need to mock finding section.
            // Parser NameSpan is for Header.
            // But properties have spans.
            // If we ask for completion below existing keys?
            // LspHelper.FindSectionAt uses heuristics.
            // Let's pick a line number > header.

            int line = doc.Sections["Config"].NameSpan.Line + 2;

            var items = LspHelper.GetCompletion(doc, line, 1);
            Assert.Contains("key2", items);
            Assert.DoesNotContain("key1", items); // Already exists
        }
    }
}
