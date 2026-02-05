using System;
using System.Collections.Generic;
using Yini.Model;

namespace Yini
{
    public class Parser
    {
        private readonly List<Token> _tokens;
        private int _position;
        private bool _isSchemaMode = false;

        public Parser(List<Token> tokens)
        {
            _tokens = tokens;
            _position = 0;
        }

        private Token Current => _position < _tokens.Count ? _tokens[_position] : _tokens[_tokens.Count - 1];

        private Token Peek(int offset = 1)
        {
            int index = _position + offset;
            return index < _tokens.Count ? _tokens[index] : _tokens[_tokens.Count - 1];
        }

        private Token Consume(TokenType type, string errorMessage)
        {
            if (Current.Type == type)
            {
                var token = Current;
                _position++;
                return token;
            }
            throw new YiniException($"{errorMessage}. Found {Current.Type} ('{Current.Value}')", Current.Span);
        }

        private bool Match(TokenType type)
        {
            if (Current.Type == type)
            {
                _position++;
                return true;
            }
            return false;
        }

        public YiniDocument Parse()
        {
            var doc = new YiniDocument();

            while (Current.Type != TokenType.EndOfFile)
            {
                if (Current.Type == TokenType.LBracket)
                {
                    ParseSection(doc);
                }
                else
                {
                    // Unexpected
                    throw new YiniException($"Unexpected token at top level: {Current}. All pairs must be in a section.", Current.Span);
                }
            }

            return doc;
        }

        private void ParseSection(YiniDocument doc)
        {
            Consume(TokenType.LBracket, "Expected '[' to start section");

            if (Current.Type == TokenType.Hash)
            {
                Consume(TokenType.Hash, "");
                var typeToken = Consume(TokenType.Identifier, "Expected identifier after #");
                Consume(TokenType.RBracket, "Expected ']'");

                string special = typeToken.Value;
                if (special == "schema")
                {
                    _isSchemaMode = true;
                    // No body for #schema header itself usually
                    // But if it has a body?
                    // Spec doesn't show body for #schema tag itself, it shows it as a flag.
                    // Check if next token is '['. If not, maybe parse body?
                    // Assuming empty body for [#schema] marker.
                }
                else
                {
                    // #define, #include
                    var section = new YiniSection("#" + special);
                    ParseBody(section);

                    // Add to doc
                    if (!_isSchemaMode)
                    {
                         if (doc.Sections.ContainsKey(section.Name))
                         {
                             // Merge
                             var existing = doc.Sections[section.Name];
                             foreach(var kv in section.Properties) existing.Properties[kv.Key] = kv.Value;
                             existing.Registry.AddRange(section.Registry);
                         }
                         else
                         {
                             doc.Sections.Add(section.Name, section);
                         }
                    }
                }
            }
            else
            {
                var nameToken = Consume(TokenType.Identifier, "Expected Section Name");
                var section = new YiniSection(nameToken.Value);

                Consume(TokenType.RBracket, "Expected ']'");

                if (Match(TokenType.Colon))
                {
                    do
                    {
                        var parent = Consume(TokenType.Identifier, "Expected Parent Name");
                        section.Parents.Add(parent.Value);
                    } while (Match(TokenType.Comma));
                }

                ParseBody(section);

                var targetDict = _isSchemaMode ? doc.Schemas : doc.Sections;

                if (targetDict.ContainsKey(section.Name))
                {
                    var existing = targetDict[section.Name];
                    foreach(var kv in section.Properties) existing.Properties[kv.Key] = kv.Value;
                    existing.Registry.AddRange(section.Registry);
                    existing.Parents.AddRange(section.Parents); // Merge parents too?
                }
                else
                {
                    targetDict.Add(section.Name, section);
                }
            }
        }

        private void ParseBody(YiniSection section)
        {
            while (Current.Type != TokenType.LBracket && Current.Type != TokenType.EndOfFile)
            {
                if (Match(TokenType.PlusAssign))
                {
                    var value = ParseExpression();
                    section.Registry.Add(value);
                }
                else if (Current.Type == TokenType.Identifier)
                {
                    // Check if it's actually a start of a section that missed bracket? No, assume key.
                    // But wait, what if we have `key` without `=`?
                    // Spec: `key = value`.
                    var keyToken = _tokens[_position]; // Don't consume yet, handled in loop
                    Consume(TokenType.Identifier, "Expected key");

                    Consume(TokenType.Assign, "Expected '='");

                    if (_isSchemaMode)
                    {
                        var schemaDef = ParseSchemaDefinition();
                        section.Properties[keyToken.Value] = schemaDef;
                    }
                    else
                    {
                        var value = ParseExpression();
                        section.Properties[keyToken.Value] = value;
                    }
                }
                else
                {
                     // Probably an error or we hit something unexpected
                     throw new YiniException($"Unexpected token in section body: {Current}", Current.Span);
                }
            }
        }

        private YiniValue ParseSchemaDefinition()
        {
            var def = new YiniSchemaDefinition();

            bool first = true;
            while (first || Match(TokenType.Comma))
            {
                first = false;

                if (Match(TokenType.Exclamation))
                {
                    def.Requirement = SchemaRequirement.Required;
                }
                else if (Match(TokenType.Question))
                {
                    def.Requirement = SchemaRequirement.Optional;
                }
                else if (Match(TokenType.Tilde))
                {
                    def.EmptyBehavior = SchemaEmptyBehavior.Ignore;
                }
                else if (Match(TokenType.Assign))
                {
                     // Default value
                     // Spec says: =1280.
                     // But wait, Empty behavior also has `=`.
                     // "Empty behavior... Assign value (=)"
                     // Spec example: `fps = ?, int, =60`
                     // This looks like default value.
                     // Does implicit `=` mean "Empty=Default"?
                     def.EmptyBehavior = SchemaEmptyBehavior.Default;
                     def.DefaultValue = ParseExpression();
                }
                else if (Current.Type == TokenType.Identifier)
                {
                    var id = Current.Value;
                    if (id == "e")
                    {
                        Advance();
                        def.EmptyBehavior = SchemaEmptyBehavior.Error;
                    }
                    else if (id == "min")
                    {
                        Advance();
                        Consume(TokenType.Assign, "Expected = after min");
                        def.Min = ParseExpression();
                    }
                    else if (id == "max")
                    {
                        Advance();
                        Consume(TokenType.Assign, "Expected = after max");
                        def.Max = ParseExpression();
                    }
                    else
                    {
                        // Assume Type Name
                        Advance();
                        def.TypeName = id;
                        // Handle array[int] syntax?
                        // Spec: array[int]
                        if (Current.Type == TokenType.LBracket)
                        {
                            Advance();
                            var subtype = Consume(TokenType.Identifier, "Expected subtype").Value;
                            Consume(TokenType.RBracket, "Expected ]");
                            def.TypeName += $"[{subtype}]";
                        }
                    }
                }
                else
                {
                    // Unexpected token in schema def
                    break;
                }
            }
            return def;
        }

        private void Advance() => _position++;

        private YiniValue ParseExpression(int precedence = 0)
        {
            var left = ParsePrimary();

            while (Current.Type != TokenType.EndOfFile)
            {
                int currentPrecedence = GetPrecedence(Current.Type);
                if (currentPrecedence < precedence) break;

                var op = Current.Type;
                _position++;
                var right = ParseExpression(currentPrecedence + 1); // +1 for left-associative

                left = new YiniBinaryExpression(left, op, right);
            }
            return left;
        }

        private int GetPrecedence(TokenType type)
        {
            switch(type)
            {
                case TokenType.Multiply:
                case TokenType.Divide:
                case TokenType.Modulo:
                    return 2;
                case TokenType.Plus:
                case TokenType.Minus:
                    return 1;
                default:
                    return -1;
            }
        }

        private YiniValue ParsePrimary()
        {
            var startToken = Current;
            YiniValue result;

            if (Match(TokenType.Minus)) // Unary Minus
            {
                var operand = ParsePrimary();
                result = new YiniUnaryExpression(TokenType.Minus, operand);
            }
            else if (Current.Type == TokenType.NumberLiteral)
            {
                var token = Consume(TokenType.NumberLiteral, "");
                if (token.Value.Contains(".")) result = new YiniFloat(float.Parse(token.Value, System.Globalization.CultureInfo.InvariantCulture));
                else result = new YiniInteger(int.Parse(token.Value));
            }
            else if (Current.Type == TokenType.StringLiteral)
            {
                result = new YiniString(Consume(TokenType.StringLiteral, "").Value);
            }
            else if (Current.Type == TokenType.BooleanLiteral)
            {
                result = new YiniBoolean(bool.Parse(Consume(TokenType.BooleanLiteral, "").Value));
            }
            else if (Current.Type == TokenType.LBracket) // Array [1, 2]
            {
                result = ParseArray();
            }
            else if (Current.Type == TokenType.LBrace) // Map {k:v}
            {
                result = ParseMap();
            }
            else if (Current.Type == TokenType.LParen) // Set or (Expr)
            {
                result = ParseParenOrSet();
            }
            else if (Current.Type == TokenType.Hash) // Color #RRGGBB
            {
                var hashToken = Consume(TokenType.Hash, "");
                string hex = "";
                // Handle split tokens (e.g. 00FF00 -> 00 (Number) + FF00 (Ident))
                // Must be on same line
                while (Current.Line == hashToken.Line && (Current.Type == TokenType.NumberLiteral || Current.Type == TokenType.Identifier))
                {
                    if (Current.Type == TokenType.NumberLiteral) hex += Consume(TokenType.NumberLiteral, "").Value;
                    else hex += Consume(TokenType.Identifier, "").Value;
                }

                if (string.IsNullOrEmpty(hex)) throw new YiniException("Expected Hex Color after #", hashToken.Span);

                result = ParseHexColor(hex);
            }
            else if (Current.Type == TokenType.At) // @name or @{...}
            {
                var atToken = Consume(TokenType.At, "");
                if (Current.Type == TokenType.LBrace)
                {
                    // @{Section.Key}
                    Consume(TokenType.LBrace, "");
                    var sec = Consume(TokenType.Identifier, "Expected Section").Value;
                    Consume(TokenType.Dot, "Expected .");
                    var key = Consume(TokenType.Identifier, "Expected Key").Value;
                    Consume(TokenType.RBrace, "");
                    result = new YiniReference($"{sec}.{key}", ReferenceType.CrossSection);
                }
                else
                {
                    // @name
                    var name = Consume(TokenType.Identifier, "Expected macro name").Value;
                    result = new YiniReference(name, ReferenceType.Macro);
                }
            }
            else if (Current.Type == TokenType.Dollar) // ${ENV}
            {
                Consume(TokenType.Dollar, "");
                Consume(TokenType.LBrace, "Expected {");
                var name = Consume(TokenType.Identifier, "Expected Env Var Name").Value;
                Consume(TokenType.RBrace, "Expected }");
                result = new YiniReference(name, ReferenceType.Environment);
            }
            else if (Current.Type == TokenType.Identifier)
            {
                var sb = new System.Text.StringBuilder();
                sb.Append(Consume(TokenType.Identifier, "").Value);

                while (Current.Type == TokenType.Dot)
                {
                    sb.Append(Consume(TokenType.Dot, "").Value);
                    sb.Append(Consume(TokenType.Identifier, "Expected identifier after dot").Value);
                }
                var id = sb.ToString();

                if (Current.Type == TokenType.LParen)
                {
                    // Constructor call: Color(), Coord(), Path(), List(), Array()
                    result = ParseConstructor(id);
                }
                else
                {
                    // Unquoted string
                    result = new YiniString(id);
                }
            }
            else
            {
                throw new YiniException($"Unexpected token in expression: {Current}", Current.Span);
            }

            // Assign Span
            if (result != null)
            {
                result.Span = startToken.Span;
            }
            return result;
        }

        private YiniValue ParseArray()
        {
            Consume(TokenType.LBracket, "");
            var items = new List<YiniValue>();
            if (Current.Type != TokenType.RBracket)
            {
                do
                {
                    items.Add(ParseExpression());
                } while (Match(TokenType.Comma));
            }
            Consume(TokenType.RBracket, "Expected ]");
            return new YiniArray(items);
        }

        private YiniValue ParseMap()
        {
            Consume(TokenType.LBrace, "");
            var map = new YiniMap();
            if (Current.Type != TokenType.RBrace)
            {
                do
                {
                    var keyToken = Consume(TokenType.Identifier, "Expected Key");
                    Consume(TokenType.Colon, "Expected :");
                    var val = ParseExpression();
                    map.Items[keyToken.Value] = val;
                } while (Match(TokenType.Comma));
            }
            Consume(TokenType.RBrace, "Expected }");
            return map;
        }

        private YiniValue ParseParenOrSet()
        {
            Consume(TokenType.LParen, "");

            // Check for empty tuple? () -> Empty set?
            if (Current.Type == TokenType.RParen)
            {
                Consume(TokenType.RParen, "");
                return new YiniSet(); // Empty
            }

            var first = ParseExpression();

            if (Current.Type == TokenType.Comma)
            {
                // Set
                var items = new List<YiniValue> { first };
                while (Match(TokenType.Comma))
                {
                    if (Current.Type == TokenType.RParen) break; // Trailing comma
                    items.Add(ParseExpression());
                }
                Consume(TokenType.RParen, "Expected )");
                return new YiniSet(items);
            }
            else
            {
                // Paren expression
                Consume(TokenType.RParen, "Expected )");
                return first;
            }
        }

        private float GetFloat(YiniValue val)
        {
            if (val is YiniFloat f) return f.Value;
            if (val is YiniInteger i) return i.Value;
            throw new Exception($"Expected number, got {val}");
        }

        private int GetInt(YiniValue val)
        {
             if (val is YiniInteger i) return i.Value;
             throw new Exception($"Expected integer, got {val}");
        }

        private YiniValue ParseConstructor(string type)
        {
            Consume(TokenType.LParen, "");
            var args = new List<YiniValue>();
            if (Current.Type != TokenType.RParen)
            {
                do
                {
                    args.Add(ParseExpression());
                } while (Match(TokenType.Comma));
            }
            Consume(TokenType.RParen, "Expected )");

            switch (type)
            {
                case "Color":
                    int r = GetInt(args[0]);
                    int g = GetInt(args[1]);
                    int b = GetInt(args[2]);
                    int a = args.Count > 3 ? GetInt(args[3]) : 255;
                    return new YiniColor(r, g, b, a);
                case "Coord":
                    float x = GetFloat(args[0]);
                    float y = GetFloat(args[1]);
                    if (args.Count > 2)
                        return new YiniCoord(x, y, GetFloat(args[2]));
                    return new YiniCoord(x, y);
                case "Path":
                    return new YiniPath(((YiniString)args[0]).Value);
                case "List":
                    return new YiniList(args);
                case "Array":
                    return new YiniArray(args);
                default:
                    throw new Exception($"Unknown type constructor: {type}");
            }
        }

        private YiniColor ParseHexColor(string hex)
        {
            // #RRGGBB
            if (hex.Length == 6)
            {
                int r = Convert.ToInt32(hex.Substring(0, 2), 16);
                int g = Convert.ToInt32(hex.Substring(2, 2), 16);
                int b = Convert.ToInt32(hex.Substring(4, 2), 16);
                return new YiniColor(r, g, b);
            }
             // Handle 3 char? #RGB?
             if (hex.Length == 3)
            {
                int r = Convert.ToInt32(new string(hex[0], 2), 16);
                int g = Convert.ToInt32(new string(hex[1], 2), 16);
                int b = Convert.ToInt32(new string(hex[2], 2), 16);
                return new YiniColor(r, g, b);
            }
            throw new Exception($"Invalid hex color length: {hex.Length} ('{hex}')");
        }
    }
}
