#include "Ast.h"
#include <stdexcept>

namespace Yini
{

// Helper to write a string to the stream
void writeString(std::ostream& os, const std::string& str) {
    size_t len = str.length();
    os.write(reinterpret_cast<const char*>(&len), sizeof(len));
    os.write(str.c_str(), len);
}

// Helper to read a string from the stream
std::string readString(std::istream& is) {
    size_t len;
    is.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (len > 1024 * 1024) { // Basic sanity check
        throw std::runtime_error("String length too large in ymeta file");
    }
    std::string str(len, '\0');
    is.read(&str[0], len);
    return str;
}

// --- Serialization ---

void ValueNode::serialize(std::ostream& os) const
{
    writeString(os, token.lexeme);
}

void KeyValuePairNode::serialize(std::ostream& os) const
{
    writeString(os, key.lexeme);
    value->serialize(os);
}

void SectionNode::serialize(std::ostream& os) const
{
    writeString(os, name.lexeme);
    size_t pairCount = pairs.size();
    os.write(reinterpret_cast<const char*>(&pairCount), sizeof(pairCount));
    for (const auto& pair : pairs)
    {
        pair->serialize(os);
    }
}


// --- Deserialization ---

std::unique_ptr<ValueNode> deserializeValue(std::istream& is) {
    Token valueToken;
    valueToken.lexeme = readString(is);
    return std::make_unique<ValueNode>(valueToken);
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