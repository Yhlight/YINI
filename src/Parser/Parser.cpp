#include "YINI/Parser.hpp"
#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <variant>

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

// Helper to apply operations, promoting to double if necessary
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
            default: break; // Percent not for doubles
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
            if (m_currentToken.type == TokenType::RightBracket) { nextToken(); }
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
            if (m_currentToken.type == TokenType::RightBracket) { nextToken(); }
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
                            YiniDocument included_doc;
                            Parser sub_parser(included_content, included_doc, m_basePath);
                            sub_parser.parse();
                            m_document.merge(included_doc);
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
            }

            if (m_currentToken.type == TokenType::RightBracket) { nextToken(); }

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
        switch (m_currentToken.type)
        {
            case TokenType::String: {
                YiniValue val;
                val.data = m_currentToken.value;
                nextToken();
                return val;
            }
            case TokenType::Boolean: {
                YiniValue val;
                val.data = (m_currentToken.value == "true");
                nextToken();
                return val;
            }
            case TokenType::LeftBracket: {
                YiniValue val;
                val.data = parseArray();
                return val;
            }
            // If it's a number, macro, or parenthesis, it's an expression
            case TokenType::Number:
            case TokenType::At:
            case TokenType::LeftParen:
            case TokenType::Minus: // For unary minus
                return parseExpression();
            default:
                nextToken();
                return {};
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
                nextToken(); // Consume '('
                result = parseExpression();
                if (m_currentToken.type == TokenType::RightParen)
                    nextToken(); // Consume ')'
                break;
            case TokenType::At:
                nextToken(); // Consume '@'
                if (m_currentToken.type == TokenType::Identifier)
                {
                    m_document.getDefine(m_currentToken.value, result);
                    nextToken();
                }
                break;
            case TokenType::Minus: // Unary minus
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

        if (m_currentToken.type == TokenType::RightBracket)
        {
            nextToken();
        }
        return arr;
    }
}