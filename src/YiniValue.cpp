#include "YiniValue.h"

// --- Helper for deep copying the variant ---
static YiniVariant deep_copy_variant(const YiniVariant& v) {
    return std::visit([](const auto& value) -> YiniVariant {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
            return std::make_unique<YiniArray>(*value);
        } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniMap>>) {
            return std::make_unique<YiniMap>(*value);
        } else {
            return value;
        }
    }, v);
}

// --- Constructors ---
YiniValue::YiniValue() : m_value(std::string("")) {}
YiniValue::YiniValue(const char* value) : m_value(std::string(value)) {}
YiniValue::YiniValue(std::string value) : m_value(std::move(value)) {}
YiniValue::YiniValue(int value) : m_value(value) {}
YiniValue::YiniValue(bool value) : m_value(value) {}
YiniValue::YiniValue(double value) : m_value(value) {}
YiniValue::YiniValue(YiniArray value) : m_value(std::make_unique<YiniArray>(std::move(value))) {}
YiniValue::YiniValue(YiniMap value) : m_value(std::make_unique<YiniMap>(std::move(value))) {}

// --- Rule of Five ---
YiniValue::~YiniValue() = default;

YiniValue::YiniValue(const YiniValue& other) : m_value(deep_copy_variant(other.m_value)) {}

YiniValue::YiniValue(YiniValue&& other) noexcept = default;

YiniValue& YiniValue::operator=(const YiniValue& other) {
    if (this != &other) {
        m_value = deep_copy_variant(other.m_value);
    }
    return *this;
}

YiniValue& YiniValue::operator=(YiniValue&& other) noexcept = default;

// --- Type checking and getting values ---
template<typename T>
bool YiniValue::is() const {
    if constexpr (std::is_same_v<T, YiniArray>) {
        return std::holds_alternative<std::unique_ptr<YiniArray>>(m_value);
    } else if constexpr (std::is_same_v<T, YiniMap>) {
        return std::holds_alternative<std::unique_ptr<YiniMap>>(m_value);
    } else {
        return std::holds_alternative<T>(m_value);
    }
}

template<typename T>
const T& YiniValue::get() const {
    if constexpr (std::is_same_v<T, YiniArray>) {
        return *std::get<std::unique_ptr<YiniArray>>(m_value);
    } else if constexpr (std::is_same_v<T, YiniMap>) {
        return *std::get<std::unique_ptr<YiniMap>>(m_value);
    } else {
        return std::get<T>(m_value);
    }
}

template<typename T>
T& YiniValue::get() {
    if constexpr (std::is_same_v<T, YiniArray>) {
        return *std::get<std::unique_ptr<YiniArray>>(m_value);
    } else if constexpr (std::is_same_v<T, YiniMap>) {
        return *std::get<std::unique_ptr<YiniMap>>(m_value);
    } else {
        return std::get<T>(m_value);
    }
}


// --- Explicit template instantiations ---
template bool YiniValue::is<std::string>() const;
template bool YiniValue::is<int>() const;
template bool YiniValue::is<bool>() const;
template bool YiniValue::is<double>() const;
template bool YiniValue::is<YiniArray>() const;
template bool YiniValue::is<YiniMap>() const;

template const std::string& YiniValue::get<std::string>() const;
template const int& YiniValue::get<int>() const;
template const bool& YiniValue::get<bool>() const;
template const double& YiniValue::get<double>() const;
template const YiniArray& YiniValue::get<YiniArray>() const;
template const YiniMap& YiniValue::get<YiniMap>() const;

template std::string& YiniValue::get<std::string>();
template int& YiniValue::get<int>();
template bool& YiniValue::get<bool>();
template double& YiniValue::get<double>();
template YiniArray& YiniValue::get<YiniArray>();
template YiniMap& YiniValue::get<YiniMap>();