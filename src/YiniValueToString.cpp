#include "YiniValueToString.hpp"
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

namespace YINI
{

static std::string collectionToString(const std::vector<YiniValue>& elements, const std::string& prefix, const std::string& suffix);
static std::string mapToString(const std::map<std::string, YiniValue>& elements);

std::string valueToString(const YiniValue& value) {
    std::stringstream ss;
    if (std::holds_alternative<std::string>(value.data)) {
        ss << std::quoted(std::get<std::string>(value.data));
    } else if (std::holds_alternative<int>(value.data)) {
        ss << std::get<int>(value.data);
    } else if (std::holds_alternative<double>(value.data)) {
        ss << std::get<double>(value.data);
    } else if (std::holds_alternative<bool>(value.data)) {
        ss << (std::get<bool>(value.data) ? "true" : "false");
    } else if (std::holds_alternative<std::unique_ptr<YiniArray>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniArray>>(value.data);
        if (ptr) ss << collectionToString(ptr->elements, "[", "]");
    } else if (std::holds_alternative<std::unique_ptr<YiniList>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniList>>(value.data);
        if (ptr) ss << collectionToString(ptr->elements, "List(", ")");
    } else if (std::holds_alternative<std::unique_ptr<YiniSet>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniSet>>(value.data);
        if (ptr) ss << collectionToString(ptr->elements, "Set(", ")");
    } else if (std::holds_alternative<std::unique_ptr<YiniMap>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniMap>>(value.data);
        if (ptr) ss << mapToString(ptr->elements);
    } else if (std::holds_alternative<std::unique_ptr<YiniPair>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniPair>>(value.data);
        if (ptr) {
            ss << "{" << std::quoted(ptr->key) << ": " << valueToString(ptr->value) << "}";
        }
    } else if (std::holds_alternative<std::unique_ptr<YiniDynaValue>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniDynaValue>>(value.data);
        if (ptr) ss << "Dyna(" << valueToString(ptr->value) << ")";
    } else if (std::holds_alternative<std::unique_ptr<YiniCoord>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniCoord>>(value.data);
        if (ptr) {
            ss << "Coord(" << ptr->x << ", " << ptr->y;
            if (ptr->is_3d) ss << ", " << ptr->z;
            ss << ")";
        }
    } else if (std::holds_alternative<std::unique_ptr<YiniColor>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniColor>>(value.data);
        if (ptr) ss << "Color(" << (int)ptr->r << ", " << (int)ptr->g << ", " << (int)ptr->b << ")";
    } else if (std::holds_alternative<std::unique_ptr<YiniPath>>(value.data)) {
        const auto& ptr = std::get<std::unique_ptr<YiniPath>>(value.data);
        if (ptr) ss << "Path(" << ptr->pathValue << ")";
    }
    return ss.str();
}

static std::string collectionToString(const std::vector<YiniValue>& elements, const std::string& prefix, const std::string& suffix) {
    std::stringstream ss;
    ss << prefix;
    for (size_t i = 0; i < elements.size(); ++i) {
        ss << valueToString(elements[i]);
        if (i < elements.size() - 1) {
            ss << ", ";
        }
    }
    ss << suffix;
    return ss.str();
}

static std::string mapToString(const std::map<std::string, YiniValue>& elements) {
    std::stringstream ss;
    ss << "{";
    size_t i = 0;
    for (const auto& [key, value] : elements) {
        ss << std::quoted(key) << ": " << valueToString(value);
        if (i < elements.size() - 1) {
            ss << ", ";
        }
        i++;
    }
    ss << "}";
    return ss.str();
}

} // namespace YINI