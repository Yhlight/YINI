#include "Strategies.h"
#include "Parser.h"

std::optional<YiniValue> StringStrategy::try_parse(Parser& parser)
{
    if (parser.match({TokenType::STRING}))
    {
        return YiniValue{parser.previous().lexeme};
    }
    return std::nullopt;
}

std::optional<YiniValue> BoolStrategy::try_parse(Parser& parser)
{
    if (parser.peek().type == TokenType::IDENTIFIER)
    {
        if (parser.peek().lexeme == "true" || parser.peek().lexeme == "false")
        {
            bool value = parser.advance().lexeme == "true";
            return YiniValue{value};
        }
    }
    return std::nullopt;
}

std::optional<YiniValue> MacroRefStrategy::try_parse(Parser& parser)
{
    if (parser.match({TokenType::AT}))
    {
        const Token& name = parser.consume(TokenType::IDENTIFIER, "Expected macro name after '@'.");
        return YiniValue{YiniMacroRef{name.lexeme}};
    }
    return std::nullopt;
}

std::optional<YiniValue> ArrayStrategy::try_parse(Parser& parser)
{
    if(parser.match({TokenType::L_BRACKET}))
    {
        // This is a temporary implementation.
        // The real implementation will call parser.parseValue() recursively.
        YiniArray array;
        if (!parser.check(TokenType::R_BRACKET))
        {
            do
            {
                array.push_back(parser.parseValue());
            } while (parser.match({TokenType::COMMA}) && !parser.check(TokenType::R_BRACKET));
        }
        parser.consume(TokenType::R_BRACKET, "Expected ']' after array elements.");
        return YiniValue{array};
    }
    return std::nullopt;
}

std::optional<YiniValue> ObjectStrategy::try_parse(Parser& parser)
{
    if (parser.check(TokenType::L_BRACE))
    {
        parser.consume(TokenType::L_BRACE, "Expected '{' to start object.");
        YiniObject obj;
        if (!parser.check(TokenType::R_BRACE))
        {
            do
            {
                const Token& key = parser.consume(TokenType::IDENTIFIER, "Expected key in object.");
                parser.consume(TokenType::COLON, "Expected ':' after key in object.");
                obj[key.lexeme] = parser.parseValue();
            } while (parser.match({TokenType::COMMA}) && !parser.check(TokenType::R_BRACE));
        }
        parser.consume(TokenType::R_BRACE, "Expected '}' after object.");
        return YiniValue{obj};
    }
    return std::nullopt;
}

std::optional<YiniValue> PathStrategy::try_parse(Parser& parser)
{
    if (parser.peek().type == TokenType::IDENTIFIER && (parser.peek().lexeme == "Path" || parser.peek().lexeme == "path"))
    {
        parser.consume(TokenType::IDENTIFIER, "Expected 'Path' or 'path'.");
        parser.consume(TokenType::L_PAREN, "Expected '(' after 'Path'.");
        const Token& path_token = parser.consume(TokenType::STRING, "Expected string for path.");
        parser.consume(TokenType::R_PAREN, "Expected ')' after path string.");
        return YiniValue{YiniPath{path_token.lexeme}};
    }
    return std::nullopt;
}

std::optional<YiniValue> CoordStrategy::try_parse(Parser& parser)
{
    if (parser.peek().type == TokenType::IDENTIFIER && (parser.peek().lexeme == "Coord" || parser.peek().lexeme == "coord"))
    {
        parser.consume(TokenType::IDENTIFIER, "Expected 'Coord' or 'coord'.");
        parser.consume(TokenType::L_PAREN, "Expected '(' after 'Coord'.");
        YiniCoord coord;

        const Token& x = parser.consume(TokenType::INTEGER, "Expected integer for x coordinate.");
        parser.consume(TokenType::COMMA, "Expected comma after x coordinate.");
        const Token& y = parser.consume(TokenType::INTEGER, "Expected integer for y coordinate.");

        coord.x = std::stod(x.lexeme);
        coord.y = std::stod(y.lexeme);

        if(parser.match({TokenType::COMMA}))
        {
            const Token& z = parser.consume(TokenType::INTEGER, "Expected integer for z coordinate.");
            coord.z = std::stod(z.lexeme);
            coord.is_3d = true;
        }

        parser.consume(TokenType::R_PAREN, "Expected ')' after coordinates.");
        return YiniValue{coord};
    }
    return std::nullopt;
}

std::optional<YiniValue> ColorStrategy::try_parse(Parser& parser)
{
    if (parser.peek().type == TokenType::IDENTIFIER && (parser.peek().lexeme == "Color" || parser.peek().lexeme == "color"))
    {
        parser.consume(TokenType::IDENTIFIER, "Expected 'color' identifier.");
        parser.consume(TokenType::L_PAREN, "Expected '(' after 'color'.");
        const Token& r = parser.consume(TokenType::INTEGER, "Expected integer for red value.");
        parser.consume(TokenType::COMMA, "Expected comma after red value.");
        const Token& g = parser.consume(TokenType::INTEGER, "Expected integer for green value.");
        parser.consume(TokenType::COMMA, "Expected comma after green value.");
        const Token& b = parser.consume(TokenType::INTEGER, "Expected integer for blue value.");
        parser.consume(TokenType::R_PAREN, "Expected ')' after color values.");
        YiniColor color;
        color.r = std::stoi(r.lexeme);
        color.g = std::stoi(g.lexeme);
        color.b = std::stoi(b.lexeme);
        return YiniValue{color};
    }
    if (parser.match({TokenType::HASH}))
    {
        const Token& hex = parser.consume(TokenType::IDENTIFIER, "Expected hex code after '#'.");
        if (hex.lexeme.length() != 6)
        {
            throw ParserError(hex, "Hex color code must be 6 characters long.");
        }
        YiniColor color;
        try
        {
            color.r = std::stoi(hex.lexeme.substr(0, 2), nullptr, 16);
            color.g = std::stoi(hex.lexeme.substr(2, 2), nullptr, 16);
            color.b = std::stoi(hex.lexeme.substr(4, 2), nullptr, 16);
        }
        catch(const std::invalid_argument& e)
        {
            throw ParserError(hex, "Invalid hex color code.");
        }
        return YiniValue{color};
    }
    return std::nullopt;
}

namespace { // Anonymous namespace for helper functions

YiniValue parse_numeric_primary(Parser& parser);
YiniValue parse_numeric_unary(Parser& parser);
YiniValue parse_numeric_factor(Parser& parser);
YiniValue parse_numeric_term(Parser& parser);
YiniValue parse_numeric_expression(Parser& parser);

YiniValue parse_numeric_primary(Parser& parser)
{
    if (parser.match({TokenType::INTEGER}))
    {
        return YiniValue{std::stoll(parser.previous().lexeme)};
    }
    if (parser.match({TokenType::FLOAT}))
    {
        return YiniValue{std::stod(parser.previous().lexeme)};
    }
    if (parser.match({TokenType::L_PAREN}))
    {
        YiniValue expr = parse_numeric_expression(parser);
        parser.consume(TokenType::R_PAREN, "Expected ')' after expression.");
        return expr;
    }

    throw ParserError(parser.peek(), "Expected a number or an expression.");
}

YiniValue parse_numeric_unary(Parser& parser)
{
    if (parser.match({TokenType::MINUS}))
    {
        Token op = parser.previous();
        YiniValue right = parse_numeric_unary(parser);
        if (std::holds_alternative<int64_t>(right.value))
        {
            return YiniValue{-std::get<int64_t>(right.value)};
        }
        else if (std::holds_alternative<double>(right.value))
        {
            return YiniValue{-std::get<double>(right.value)};
        }
        else
        {
            throw ParserError(op, "Operand must be a number.");
        }
    }
    return parse_numeric_primary(parser);
}

YiniValue parse_numeric_factor(Parser& parser)
{
    return parse_numeric_unary(parser);
}

YiniValue parse_numeric_term(Parser& parser)
{
    YiniValue left = parse_numeric_factor(parser);
    while (parser.match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT}))
    {
        Token op = parser.previous();
        YiniValue right = parse_numeric_factor(parser);
        if (std::holds_alternative<int64_t>(left.value) && std::holds_alternative<int64_t>(right.value))
        {
            if (op.type == TokenType::STAR)
                left = YiniValue{std::get<int64_t>(left.value) * std::get<int64_t>(right.value)};
            else if (op.type == TokenType::SLASH)
                left = YiniValue{std::get<int64_t>(left.value) / std::get<int64_t>(right.value)};
            else
                left = YiniValue{std::get<int64_t>(left.value) % std::get<int64_t>(right.value)};
        }
        else if ((std::holds_alternative<double>(left.value) || std::holds_alternative<int64_t>(left.value)) && (std::holds_alternative<double>(right.value) || std::holds_alternative<int64_t>(right.value)))
        {
            double left_val = std::holds_alternative<double>(left.value) ? std::get<double>(left.value) : static_cast<double>(std::get<int64_t>(left.value));
            double right_val = std::holds_alternative<double>(right.value) ? std::get<double>(right.value) : static_cast<double>(std::get<int64_t>(right.value));
            if (op.type == TokenType::STAR)
                left = YiniValue{left_val * right_val};
            else if (op.type == TokenType::SLASH)
                left = YiniValue{left_val / right_val};
            else
                throw ParserError(op, "Modulo operator requires integer operands.");
        }
        else
        {
            throw ParserError(op, "Operands must be numbers.");
        }
    }
    return left;
}

YiniValue parse_numeric_expression(Parser& parser)
{
    YiniValue left = parse_numeric_term(parser);
    while (parser.match({TokenType::PLUS, TokenType::MINUS}))
    {
        Token op = parser.previous();
        YiniValue right = parse_numeric_term(parser);
        if (std::holds_alternative<int64_t>(left.value) && std::holds_alternative<int64_t>(right.value))
        {
            if (op.type == TokenType::PLUS)
                left = YiniValue{std::get<int64_t>(left.value) + std::get<int64_t>(right.value)};
            else
                left = YiniValue{std::get<int64_t>(left.value) - std::get<int64_t>(right.value)};
        }
        else if ((std::holds_alternative<double>(left.value) || std::holds_alternative<int64_t>(left.value)) && (std::holds_alternative<double>(right.value) || std::holds_alternative<int64_t>(right.value)))
        {
            double left_val = std::holds_alternative<double>(left.value) ? std::get<double>(left.value) : static_cast<double>(std::get<int64_t>(left.value));
            double right_val = std::holds_alternative<double>(right.value) ? std::get<double>(right.value) : static_cast<double>(std::get<int64_t>(right.value));
            if (op.type == TokenType::PLUS)
                left = YiniValue{left_val + right_val};
            else
                left = YiniValue{left_val - right_val};
        }
        else
        {
            throw ParserError(op, "Operands must be numbers.");
        }
    }
    return left;
}

} // anonymous namespace

std::optional<YiniValue> NumericExpressionStrategy::try_parse(Parser& parser)
{
    // This strategy triggers if the token is a number or a minus sign (for unary negation) or a parenthesis.
    if (parser.check(TokenType::INTEGER) || parser.check(TokenType::FLOAT) || parser.check(TokenType::MINUS) || parser.check(TokenType::L_PAREN))
    {
        return parse_numeric_expression(parser);
    }
    return std::nullopt;
}
