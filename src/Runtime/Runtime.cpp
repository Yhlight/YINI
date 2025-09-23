#include "Runtime.h"
#include <stdexcept>
#include <fstream>
#include <vector>

namespace Yini
{
    // --- Binary I/O Helpers ---
    template<typename T>
    void write_binary(std::ostream& os, const T& value) {
        os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template<typename T>
    void read_binary(std::istream& is, T& value) {
        is.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    void write_string(std::ostream& os, const std::string& str) {
        uint16_t len = static_cast<uint16_t>(str.length());
        write_binary(os, len);
        os.write(str.c_str(), len);
    }

    std::string read_string(std::istream& is) {
        uint16_t len = 0;
        read_binary(is, len);
        std::vector<char> buffer(len);
        is.read(buffer.data(), len);
        return std::string(buffer.begin(), buffer.end());
    }

    enum class ValueType : uint8_t {
        NIL = 0, INTEGER = 1, FLOAT = 2, BOOLEAN = 3, STRING = 4
    };

    void write_value(std::ostream& os, const Value& value)
    {
        std::visit(overloaded {
            [&](std::monostate) { write_binary<uint8_t>(os, (uint8_t)ValueType::NIL); },
            [&](Integer i) { write_binary(os, (uint8_t)ValueType::INTEGER); write_binary(os, i); },
            [&](Float f) { write_binary(os, (uint8_t)ValueType::FLOAT); write_binary(os, f); },
            [&](Boolean b) { write_binary(os, (uint8_t)ValueType::BOOLEAN); write_binary(os, b); },
            [&](const String& s) { write_binary(os, (uint8_t)ValueType::STRING); uint32_t len = s.length(); write_binary(os, len); os.write(s.c_str(), len); },
            [&](const auto&) { /* Other types not supported yet */ }
        }, value.data);
    }

    std::shared_ptr<Value> read_value(std::istream& is)
    {
        auto val = std::make_shared<Value>();
        uint8_t type_byte;
        read_binary(is, type_byte);
        ValueType type = static_cast<ValueType>(type_byte);

        switch(type) {
            case ValueType::INTEGER: { Integer i; read_binary(is, i); val->data = i; break; }
            case ValueType::FLOAT: { Float f; read_binary(is, f); val->data = f; break; }
            case ValueType::BOOLEAN: { Boolean b; read_binary(is, b); val->data = b; break; }
            case ValueType::STRING: { uint32_t len; read_binary(is, len); std::vector<char> buf(len); is.read(buf.data(), len); val->data = String(buf.begin(), buf.end()); break; }
            case ValueType::NIL:
            default:
                val->data = std::monostate();
                break;
        }
        return val;
    }


    // --- YiniRuntime Implementation ---

    void YiniRuntime::evaluate(Ast::YiniDocument* doc)
    {
        // First pass: handle all definitions
        for (const auto& stmt : doc->statements)
        {
            if (auto* section = dynamic_cast<Ast::Section*>(stmt.get()))
            {
                if (section->name->value == "#define")
                {
                    m_currentSectionName = "#define";
                    for (const auto& section_stmt : section->statements)
                    {
                        visit(section_stmt.get());
                    }
                }
            }
        }

        // Second pass: evaluate everything else
        for (const auto& stmt : doc->statements)
        {
            if (auto* section = dynamic_cast<Ast::Section*>(stmt.get()))
            {
                if (section->name->value == "#define")
                {
                    continue;
                }
            }
            visit(stmt.get());
        }
    }

    std::shared_ptr<Value> YiniRuntime::getValue(const std::string& sectionName, const std::string& key) const
    {
        if (m_sections.count(sectionName))
        {
            const auto& section = m_sections.at(sectionName);
            if (section.count(key))
            {
                return section.at(key);
            }
        }
        return nullptr;
    }

    std::string YiniRuntime::dump() const
    {
        std::stringstream ss;
        ss << "--- Defines ---\n";
        for (const auto& [key, val] : m_defines)
        {
            ss << "  " << key << " = " << val->toString() << "\n";
        }
        ss << "--- Sections ---\n";
        for (const auto& [sectionName, sectionData] : m_sections)
        {
            ss << "[" << sectionName << "]\n";
            for (const auto& [key, val] : sectionData)
            {
                ss << "  " << key << " = " << val->toString() << "\n";
            }
        }
        return ss.str();
    }

    bool YiniRuntime::serialize(const std::string& filepath) const
    {
        std::ofstream os(filepath, std::ios::binary);
        if (!os) return false;

        // Header
        os.write("YINI", 4);
        write_binary<uint8_t>(os, 1); // Version

        // Defines
        write_binary<uint8_t>(os, 0x01); // Defines chunk type
        write_binary<uint32_t>(os, m_defines.size());
        for (const auto& [key, val] : m_defines) {
            write_string(os, key);
            write_value(os, *val);
        }

        // Sections
        write_binary<uint8_t>(os, 0x02); // Sections chunk type
        write_binary<uint32_t>(os, m_sections.size());
        for (const auto& [sectionName, sectionData] : m_sections) {
            write_string(os, sectionName);
            write_binary<uint32_t>(os, sectionData.size());
            for (const auto& [key, val] : sectionData) {
                write_string(os, key);
                write_value(os, *val);
            }
        }
        return true;
    }

    bool YiniRuntime::deserialize(const std::string& filepath)
    {
        std::ifstream is(filepath, std::ios::binary);
        if (!is) return false;

        char header[4];
        is.read(header, 4);
        if (std::string(header, 4) != "YINI") return false;

        uint8_t version;
        read_binary(is, version);
        if (version != 1) return false;

        m_defines.clear();
        m_sections.clear();

        while(is.peek() != EOF) {
            uint8_t chunkType;
            read_binary(is, chunkType);
            if (chunkType == 0x01) { // Defines
                uint32_t count;
                read_binary(is, count);
                for(uint32_t i = 0; i < count; ++i) {
                    std::string key = read_string(is);
                    m_defines[key] = read_value(is);
                }
            } else if (chunkType == 0x02) { // Sections
                uint32_t sectionCount;
                read_binary(is, sectionCount);
                for (uint32_t i = 0; i < sectionCount; ++i) {
                    std::string sectionName = read_string(is);
                    m_sections[sectionName] = SectionData();
                    uint32_t kvpCount;
                    read_binary(is, kvpCount);
                    for (uint32_t j = 0; j < kvpCount; ++j) {
                        std::string key = read_string(is);
                        m_sections[sectionName][key] = read_value(is);
                    }
                }
            }
        }
        return true;
    }

    SectionData& YiniRuntime::getCurrentSection()
    {
        if (m_currentSectionName != "#define" && m_sections.find(m_currentSectionName) == m_sections.end())
        {
            m_sections[m_currentSectionName] = SectionData();
        }
        return m_sections.at(m_currentSectionName);
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Node* node)
    {
        if (!node) return nullptr;
        if (auto* n = dynamic_cast<Ast::Section*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::KeyValuePair*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::QuickRegister*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::Expression*>(node)) return visit(n);
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Expression* node)
    {
        if (!node) return nullptr;
        if (auto* n = dynamic_cast<Ast::Identifier*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::IntegerLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::FloatLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::BooleanLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::StringLiteral*>(node)) return visit(n);
        if (auto* n = dynamic_cast<Ast::InfixExpression*>(node)) return visit(n);
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Section* node)
    {
        m_currentSectionName = node->name->value;
        m_sections[m_currentSectionName] = SectionData();
        for (const auto& stmt : node->statements)
        {
            visit(stmt.get());
        }
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::KeyValuePair* node)
    {
        auto value = visit(node->value.get());
        if (value)
        {
            if (m_currentSectionName == "#define")
            {
                m_defines[node->key->value] = value;
            }
            else
            {
                getCurrentSection()[node->key->value] = value;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::QuickRegister* node)
    {
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::Identifier* node)
    {
        if (m_currentSectionName != "#define" && m_sections.count(m_currentSectionName))
        {
             if (getCurrentSection().count(node->value))
             {
                 return getCurrentSection().at(node->value);
             }
        }
        if (m_defines.count(node->value))
        {
            return m_defines.at(node->value);
        }
        return nullptr;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::IntegerLiteral* node)
    {
        auto val = std::make_shared<Value>();
        val->data.emplace<Integer>(node->value);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::FloatLiteral* node)
    {
        auto val = std::make_shared<Value>();
        val->data.emplace<Float>(node->value);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::BooleanLiteral* node)
    {
        auto val = std::make_shared<Value>();
        val->data.emplace<Boolean>(node->value);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::StringLiteral* node)
    {
        auto val = std::make_shared<Value>();
        val->data.emplace<String>(node->value);
        return val;
    }

    std::shared_ptr<Value> YiniRuntime::visit(Ast::InfixExpression* node)
    {
        auto left = visit(node->left.get());
        auto right = visit(node->right.get());
        if (!left || !right) return nullptr;

        auto result = std::make_shared<Value>();
        if (std::holds_alternative<Float>(left->data) || std::holds_alternative<Float>(right->data))
        {
            double leftVal = std::holds_alternative<Float>(left->data) ? std::get<Float>(left->data) : (double)std::get<Integer>(left->data);
            double rightVal = std::holds_alternative<Float>(right->data) ? std::get<Float>(right->data) : (double)std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Float>(leftVal + rightVal);
            else if (node->op == "-") result->data.emplace<Float>(leftVal - rightVal);
            else if (node->op == "*") result->data.emplace<Float>(leftVal * rightVal);
            else if (node->op == "/") result->data.emplace<Float>(leftVal / rightVal);
        }
        else if (std::holds_alternative<Integer>(left->data) && std::holds_alternative<Integer>(right->data))
        {
            long long leftVal = std::get<Integer>(left->data);
            long long rightVal = std::get<Integer>(right->data);
            if (node->op == "+") result->data.emplace<Integer>(leftVal + rightVal);
            else if (node->op == "-") result->data.emplace<Integer>(leftVal - rightVal);
            else if (node->op == "*") result->data.emplace<Integer>(leftVal * rightVal);
            else if (node->op == "/") result->data.emplace<Integer>(leftVal / rightVal);
            else if (node->op == "%") result->data.emplace<Integer>(leftVal % rightVal);
        }

        return result;
    }
}
