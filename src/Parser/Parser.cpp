#include "YINI/Parser.hpp"
#include "YINI/YiniException.hpp"
#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <variant>
#include <algorithm>
#include <cctype>

static std::string read_file_content_internal(const std::string& path) {
    std::ifstream t(path);
    if (!t.is_open()) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

namespace { // Anonymous namespace for helpers

bool is_numeric(const YINI::YiniValue& val) {
    return std::holds_alternative<int>(val.data) || std::holds_alternative<double>(val.data);
}

YINI::YiniValue apply_op(const YINI::YiniValue& left, const YINI::YiniValue& right, YINI::TokenType op) {
    if (!is_numeric(left) || !is_numeric(right)) return {};

    YINI::YiniValue result;
    bool use_double = std::holds_alternative<double>(left.data) || std::holds_alternative<double>(right.data);

    double left_d = std::holds_alternative<int>(left.data) ? static_cast<double>(std::get<int>(left.data)) : std::get<double>(left.data);
    double right_d = std::holds_alternative<int>(right.data) ? static_cast<double>(std::get<int>(right.data)) : std::get<double>(right.data);

    if (use_double) {
        switch (op) {
            case YINI::TokenType::Plus: result.data = left_d + right_d; break;
            case YINI::TokenType::Minus: result.data = left_d - right_d; break;
            case YINI::TokenType::Star: result.data = left_d * right_d; break;
            case YINI::TokenType::Slash: result.data = right_d != 0 ? left_d / right_d : 0.0; break;
            default: break;
        }
    } else {
        int left_i = std::get<int>(left.data);
        int right_i = std::get<int>(right.data);
        switch (op) {
            case YINI::TokenType::Plus: result.data = left_i + right_i; break;
            case YINI::TokenType::Minus: result.data = left_i - right_i; break;
            case YINI::TokenType::Star: result.data = left_i * right_i; break;
            case YINI::TokenType::Slash: result.data = right_i != 0 ? left_i / right_i : 0; break;
            case YINI::TokenType::Percent: result.data = right_i != 0 ? left_i % right_i : 0; break;
            default: break;
        }
    }
    return result;
}

} // end anonymous namespace

namespace YINI
{
    Parser::Parser(const std::string& content, YiniDocument& document, const std::string& basePath)
        : m_lexer(content), m_document(document), m_basePath(basePath)
    {
        nextToken();
    }

    void Parser::nextToken()
    {
        m_currentToken = m_lexer.getNextToken();
    }

    void Parser::parse()
    {
        while (m_currentToken.type != TokenType::Eof)
        {
            if (m_currentToken.type == TokenType::LeftBracket)
            {
                parseSection();
            }
            else
            {
                if (m_currentToken.type != TokenType::Eof)
                {
                     throw YiniException("Unexpected token at root level.", m_currentToken.line, m_currentToken.column);
                }
                nextToken();
            }
        }
    }

    void Parser::parseSection()
    {
        nextToken();

        bool is_define_section = false;
        bool is_include_section = false;

        if (m_currentToken.type == TokenType::Hash)
        {
            nextToken();
            if (m_currentToken.type == TokenType::Identifier && m_currentToken.value == "define")
            {
                is_define_section = true;
                nextToken();
            }
            else if (m_currentToken.type == TokenType::Identifier && m_currentToken.value == "include")
            {
                is_include_section = true;
                nextToken();
            }
        }

        if (is_define_section)
        {
            if (m_currentToken.type != TokenType::RightBracket) {
                 throw YiniException("Expected ']' to close #define directive.", m_currentToken.line, m_currentToken.column);
            }
            nextToken();
            while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::Eof)
            {
                if (m_currentToken.type == TokenType::Identifier) {
                    std::string key = m_currentToken.value;
                    nextToken();
                    if (m_currentToken.type == TokenType::Equals) {
                        nextToken();
                        m_document.addDefine(key, parseValue());
                    } else { nextToken(); }
                } else { nextToken(); }
            }
        }
        else if (is_include_section)
        {
            if (m_currentToken.type != TokenType::RightBracket) {
                 throw YiniException("Expected ']' to close #include directive.", m_currentToken.line, m_currentToken.column);
            }
            nextToken();
            while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::Eof)
            {
                if (m_currentToken.type == TokenType::PlusEquals)
                {
                    nextToken();
                    if (m_currentToken.type == TokenType::Identifier)
                    {
                        std::string file_to_include = m_currentToken.value;
                        std::string full_path = m_basePath + "/" + file_to_include;
                        std::string included_content = read_file_content_internal(full_path);
                        if (!included_content.empty())
                        {
                            Parser sub_parser(included_content, m_document, m_basePath);
                            sub_parser.parse();
                        }
                        nextToken();
                    } else { nextToken(); }
                } else { nextToken(); }
            }
        }
        else
        {
            std::string section_name_val;
            if (m_currentToken.type == TokenType::Identifier)
            {
                section_name_val = m_currentToken.value;
                nextToken();
            } else {
                throw YiniException("Invalid section name.", m_currentToken.line, m_currentToken.column);
            }

            YiniSection* section = m_document.getOrCreateSection(section_name_val);

            if (m_currentToken.type == TokenType::Colon)
            {
                nextToken();
                while (m_currentToken.type == TokenType::Identifier)
                {
                    section->inheritedSections.push_back(m_currentToken.value);
                    nextToken();
                    if (m_currentToken.type == TokenType::Comma) { nextToken(); }
                    else { break; }
                }
            }

            if (m_currentToken.type != TokenType::RightBracket)
            {
                throw YiniException("Expected ']' to close section header.", m_currentToken.line, m_currentToken.column);
            }
            nextToken();

            while (m_currentToken.type != TokenType::LeftBracket && m_currentToken.type != TokenType::Eof)
            {
                if (m_currentToken.type == TokenType::Identifier)
                {
                    parseKeyValuePair(*section);
                }
                else if (m_currentToken.type == TokenType::PlusEquals)
                {
                    parseQuickRegistration(*section);
                }
                else
                {
                    nextToken();
                }
            }
        }
    }

    void Parser::parseKeyValuePair(YiniSection& section)
    {
        YiniKeyValuePair pair;
        pair.key = m_currentToken.value;
        nextToken();

        if (m_currentToken.type == TokenType::Equals)
        {
            nextToken();
            pair.value = parseValue();

            auto it = std::find_if(section.pairs.begin(), section.pairs.end(),
                [&](const YiniKeyValuePair& p) { return p.key == pair.key; });

            if (it != section.pairs.end())
            {
                it->value = pair.value;
            }
            else
            {
                section.pairs.push_back(std::move(pair));
            }
        }
    }

    void Parser::parseQuickRegistration(YiniSection& section)
    {
        nextToken();
        section.registrationList.push_back(parseValue());
    }

    YiniValue Parser::parseValue()
    {
        YiniValue val;
        switch (m_currentToken.type)
        {
            case TokenType::String:
                val.data = m_currentToken.value;
                nextToken();
                return val;
            case TokenType::Boolean:
                val.data = (m_currentToken.value == "true");
                nextToken();
                return val;
            case TokenType::LeftBracket:
                val.data = parseArray();
                return val;
            case TokenType::LeftBrace:
                nextToken(); // Consume '{'
                if (m_currentToken.type == TokenType::LeftBrace)
                {
                    // Map starts with {{
                    val.data = parseMap();
                }
                else
                {
                    // Pair starts with {
                    val.data = parsePair();
                }
                return val;
            case TokenType::HexColor:
                val.data = parseColor();
                return val;
            case TokenType::Identifier: {
                std::string id_val = m_currentToken.value;
                std::transform(id_val.begin(), id_val.end(), id_val.begin(),
                               [](unsigned char c){ return std::tolower(c); });

                if (id_val == "dyna") {
                    nextToken();
                    if (m_currentToken.type != TokenType::LeftParen) throw YiniException("Expected '(' after Dyna.", m_currentToken.line, m_currentToken.column);
                    nextToken();
                    auto dyna_val = std::make_unique<YiniDynaValue>();
                    dyna_val->value = parseValue();
                    if (m_currentToken.type != TokenType::RightParen) throw YiniException("Expected ')' to close Dyna expression.", m_currentToken.line, m_currentToken.column);
                    nextToken();
                    val.data = std::move(dyna_val);
                    return val;
                }
                if (id_val == "coord") {
                    val.data = parseCoord();
                    return val;
                }
                if (id_val == "color") {
                    val.data = parseColor();
                    return val;
                }
                if (id_val == "path") {
                    val.data = parsePath();
                    return val;
                }
            }
            case TokenType::Number:
            case TokenType::At:
            case TokenType::LeftParen:
            case TokenType::Minus:
                return parseExpression();
            default:
                throw YiniException("Unexpected token when parsing value.", m_currentToken.line, m_currentToken.column);
        }
    }

    YiniValue Parser::parseFactor()
    {
        YiniValue result;
        switch (m_currentToken.type)
        {
            case TokenType::Number:
                if (m_currentToken.value.find('.') != std::string::npos)
                    result.data = std::stod(m_currentToken.value);
                else
                    result.data = std::stoi(m_currentToken.value);
                nextToken();
                break;
            case TokenType::LeftParen:
                nextToken();
                result = parseExpression();
                if (m_currentToken.type != TokenType::RightParen)
                    throw YiniException("Expected ')' to close expression.", m_currentToken.line, m_currentToken.column);
                nextToken();
                break;
            case TokenType::At:
                nextToken();
                if (m_currentToken.type == TokenType::Identifier)
                {
                    m_document.getDefine(m_currentToken.value, result);
                    nextToken();
                }
                break;
            case TokenType::Minus:
                nextToken();
                result = parseFactor();
                if(is_numeric(result))
                {
                     if (std::holds_alternative<double>(result.data))
                        result.data = -std::get<double>(result.data);
                     else
                        result.data = -std::get<int>(result.data);
                }
                break;
            default:
                nextToken();
                break;
        }
        return result;
    }

    YiniValue Parser::parseTerm()
    {
        YiniValue result = parseFactor();
        while (m_currentToken.type == TokenType::Star || m_currentToken.type == TokenType::Slash || m_currentToken.type == TokenType::Percent)
        {
            Token op = m_currentToken;
            nextToken();
            YiniValue right = parseFactor();
            result = apply_op(result, right, op.type);
        }
        return result;
    }

    YiniValue Parser::parseExpression()
    {
        YiniValue result = parseTerm();
        while (m_currentToken.type == TokenType::Plus || m_currentToken.type == TokenType::Minus)
        {
            Token op = m_currentToken;
            nextToken();
            YiniValue right = parseTerm();
            result = apply_op(result, right, op.type);
        }
        return result;
    }

    std::unique_ptr<YiniArray> Parser::parseArray()
    {
        auto arr = std::make_unique<YiniArray>();
        nextToken();

        while (m_currentToken.type != TokenType::RightBracket && m_currentToken.type != TokenType::Eof)
        {
            arr->elements.push_back(parseValue());

            if (m_currentToken.type == TokenType::Comma)
            {
                nextToken();
            }
        }

        if (m_currentToken.type != TokenType::RightBracket)
        {
            throw YiniException("Expected ']' to close array.", m_currentToken.line, m_currentToken.column);
        }
        nextToken();
        return arr;
    }

    std::unique_ptr<YiniCoord> Parser::parseCoord() {
        nextToken(); // consume 'Coord'
        if (m_currentToken.type != TokenType::LeftParen) throw YiniException("Expected '(' after Coord.", m_currentToken.line, m_currentToken.column);
        nextToken(); // consume '('

        auto coord = std::make_unique<YiniCoord>();

        YiniValue x_val = parseExpression();
        if (std::holds_alternative<int>(x_val.data)) coord->x = std::get<int>(x_val.data);
        else if (std::holds_alternative<double>(x_val.data)) coord->x = std::get<double>(x_val.data);
        else throw YiniException("Coord parameters must be numeric.", m_currentToken.line, m_currentToken.column);

        if (m_currentToken.type != TokenType::Comma) throw YiniException("Expected ',' in Coord.", m_currentToken.line, m_currentToken.column);
        nextToken();
        YiniValue y_val = parseExpression();
        if (std::holds_alternative<int>(y_val.data)) coord->y = std::get<int>(y_val.data);
        else if (std::holds_alternative<double>(y_val.data)) coord->y = std::get<double>(y_val.data);
        else throw YiniException("Coord parameters must be numeric.", m_currentToken.line, m_currentToken.column);

        if (m_currentToken.type == TokenType::Comma) {
            nextToken();
            YiniValue z_val = parseExpression();
            if (std::holds_alternative<int>(z_val.data)) coord->z = std::get<int>(z_val.data);
            else if (std::holds_alternative<double>(z_val.data)) coord->z = std::get<double>(z_val.data);
            else throw YiniException("Coord parameters must be numeric.", m_currentToken.line, m_currentToken.column);
            coord->is_3d = true;
        } else {
            coord->z = 0;
            coord->is_3d = false;
        }

        if (m_currentToken.type != TokenType::RightParen) throw YiniException("Expected ')' to close Coord expression.", m_currentToken.line, m_currentToken.column);
        nextToken();
        return coord;
    }

    std::unique_ptr<YiniColor> Parser::parseColor() {
        if (m_currentToken.type == TokenType::HexColor) {
            auto color = std::make_unique<YiniColor>();
            unsigned int r, g, b;
            sscanf(m_currentToken.value.c_str(), "%2x%2x%2x", &r, &g, &b);
            color->r = r;
            color->g = g;
            color->b = b;
            nextToken();
            return color;
        }

        nextToken(); // consume 'Color'
        if (m_currentToken.type != TokenType::LeftParen) throw YiniException("Expected '(' after Color.", m_currentToken.line, m_currentToken.column);
        nextToken(); // consume '('

        auto color = std::make_unique<YiniColor>();

        YiniValue r_val = parseExpression();
        if (!std::holds_alternative<int>(r_val.data)) throw YiniException("Color parameters must be integers.", m_currentToken.line, m_currentToken.column);
        color->r = std::get<int>(r_val.data);

        if (m_currentToken.type != TokenType::Comma) throw YiniException("Expected ',' in Color.", m_currentToken.line, m_currentToken.column);
        nextToken();
        YiniValue g_val = parseExpression();
        if (!std::holds_alternative<int>(g_val.data)) throw YiniException("Color parameters must be integers.", m_currentToken.line, m_currentToken.column);
        color->g = std::get<int>(g_val.data);

        if (m_currentToken.type != TokenType::Comma) throw YiniException("Expected ',' in Color.", m_currentToken.line, m_currentToken.column);
        nextToken();
        YiniValue b_val = parseExpression();
        if (!std::holds_alternative<int>(b_val.data)) throw YiniException("Color parameters must be integers.", m_currentToken.line, m_currentToken.column);
        color->b = std::get<int>(b_val.data);

        if (m_currentToken.type != TokenType::RightParen) throw YiniException("Expected ')' to close Color expression.", m_currentToken.line, m_currentToken.column);
        nextToken();
        return color;
    }

    std::unique_ptr<YiniPath> Parser::parsePath() {
        nextToken(); // consume 'Path'
        if (m_currentToken.type != TokenType::LeftParen) throw YiniException("Expected '(' after Path.", m_currentToken.line, m_currentToken.column);
        nextToken(); // consume '('

        auto path = std::make_unique<YiniPath>();

        std::string path_str;
        while (m_currentToken.type != TokenType::RightParen && m_currentToken.type != TokenType::Eof) {
            path_str += m_currentToken.value;
            nextToken();
        }
        path->path_value = path_str;

        if (m_currentToken.type != TokenType::RightParen) throw YiniException("Expected ')' to close Path expression.", m_currentToken.line, m_currentToken.column);
        nextToken();
        return path;
    }

    std::unique_ptr<YiniPair> Parser::parsePair()
    {
        auto pair = std::make_unique<YiniPair>();

        if (m_currentToken.type != TokenType::Identifier)
        {
            throw YiniException("Expected identifier for pair key.", m_currentToken.line, m_currentToken.column);
        }
        pair->key = m_currentToken.value;
        nextToken();

        if (m_currentToken.type != TokenType::Colon)
        {
            throw YiniException("Expected ':' after pair key.", m_currentToken.line, m_currentToken.column);
        }
        nextToken(); // Consume ':'

        pair->value = parseValue();

        if (m_currentToken.type != TokenType::RightBrace)
        {
            throw YiniException("Expected '}' to close pair.", m_currentToken.line, m_currentToken.column);
        }
        nextToken(); // Consume '}'

        return pair;
    }

    std::unique_ptr<YiniMap> Parser::parseMap()
    {
        auto map = std::make_unique<YiniMap>();
        nextToken(); // consume the inner '{'

        while (m_currentToken.type != TokenType::RightBrace)
        {
            if (m_currentToken.type != TokenType::Identifier)
            {
                throw YiniException("Expected identifier for map key.", m_currentToken.line, m_currentToken.column);
            }
            std::string key = m_currentToken.value;
            nextToken();

            if (m_currentToken.type != TokenType::Colon)
            {
                throw YiniException("Expected ':' after map key.", m_currentToken.line, m_currentToken.column);
            }
            nextToken(); // Consume ':'

            map->elements[key] = parseValue();

            if (m_currentToken.type == TokenType::Comma)
            {
                nextToken();
            }
            else if (m_currentToken.type == TokenType::RightBrace)
            {
                break;
            }
            else
            {
                throw YiniException("Expected ',' or '}' in map definition.", m_currentToken.line, m_currentToken.column);
            }
        }

        if (m_currentToken.type != TokenType::RightBrace)
        {
            throw YiniException("Expected '}' to close map.", m_currentToken.line, m_currentToken.column);
        }
        nextToken(); // consume inner '}'

        if (m_currentToken.type != TokenType::RightBrace)
        {
            throw YiniException("Expected '}}' to close map.", m_currentToken.line, m_currentToken.column);
        }
        nextToken(); // consume outer '}'

        return map;
    }
}