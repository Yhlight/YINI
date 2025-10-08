#include "Ast.h"
#include <stdexcept>

namespace Yini
{

// --- Serialization Helpers ---

void writeString(std::ostream& os, const std::string& str) {
    size_t len = str.length();
    os.write(reinterpret_cast<const char*>(&len), sizeof(len));
    os.write(str.c_str(), len);
}

// --- Deserialization Helpers ---

std::string readString(std::istream& is) {
    size_t len;
    is.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (is.gcount() != sizeof(len) || len > 1024 * 1024) { // Basic sanity check
        throw std::runtime_error("Invalid string length in ymeta file");
    }
    std::string str(len, '\0');
    is.read(&str[0], len);
    if (is.gcount() != len) {
        throw std::runtime_error("Could not read full string from ymeta file");
    }
    return str;
}


// --- Serialization Implementations ---

void IdentifierValue::serialize(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&token.type), sizeof(token.type));
    writeString(os, token.lexeme);
}

void StringValue::serialize(std::ostream& os) const {
    writeString(os, value);
}

void NumberValue::serialize(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void BoolValue::serialize(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void ArrayValue::serialize(std::ostream& os) const {
    size_t count = elements.size();
    os.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& elem : elements) {
        ValueType type = elem->getType();
        os.write(reinterpret_cast<const char*>(&type), sizeof(type));
        elem->serialize(os);
    }
}

void KeyValuePairNode::serialize(std::ostream& os) const {
    writeString(os, key.lexeme);
    ValueType type = value->getType();
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));
    value->serialize(os);
}

void SectionNode::serialize(std::ostream& os) const {
    writeString(os, name.lexeme);
    size_t pairCount = pairs.size();
    os.write(reinterpret_cast<const char*>(&pairCount), sizeof(pairCount));
    for (const auto& pair : pairs) {
        pair->serialize(os);
    }
}


// --- Deserialization Implementations ---

std::unique_ptr<Value> deserializeValue(std::istream& is) {
    ValueType type;
    is.read(reinterpret_cast<char*>(&type), sizeof(type));
    if (is.gcount() != sizeof(type)) {
        throw std::runtime_error("Could not read value type from ymeta file");
    }

    switch (type) {
        case ValueType::Identifier: {
            Token t;
            is.read(reinterpret_cast<char*>(&t.type), sizeof(t.type));
            t.lexeme = readString(is);
            return std::make_unique<IdentifierValue>(t);
        }
        case ValueType::String: {
            return std::make_unique<StringValue>(readString(is));
        }
        case ValueType::Number: {
            double val;
            is.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_unique<NumberValue>(val);
        }
        case ValueType::Bool: {
            bool val;
            is.read(reinterpret_cast<char*>(&val), sizeof(val));
            return std::make_unique<BoolValue>(val);
        }
        case ValueType::Array: {
            auto array = std::make_unique<ArrayValue>();
            size_t count;
            is.read(reinterpret_cast<char*>(&count), sizeof(count));
            for (size_t i = 0; i < count; ++i) {
                array->elements.push_back(deserializeValue(is));
            }
            return array;
        }
        default:
            throw std::runtime_error("Unknown value type in ymeta file");
    }
}

std::unique_ptr<KeyValuePairNode> deserializeKeyValuePair(std::istream& is) {
    Token keyToken;
    keyToken.lexeme = readString(is);
    auto value = deserializeValue(is);
    return std::make_unique<KeyValuePairNode>(keyToken, std::move(value));
}

std::unique_ptr<SectionNode> deserializeSection(std::istream& is) {
    Token nameToken;
    nameToken.lexeme = readString(is);
    auto section = std::make_unique<SectionNode>(nameToken);

    size_t pairCount;
    is.read(reinterpret_cast<char*>(&pairCount), sizeof(pairCount));
    for (size_t i = 0; i < pairCount; ++i) {
        section->pairs.push_back(deserializeKeyValuePair(is));
    }
    return section;
}

} // namespace Yini