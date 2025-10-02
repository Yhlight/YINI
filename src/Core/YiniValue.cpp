#include "YiniValue.h"
#include "DynaValue.h"

namespace YINI
{
    // Default constructor
    YiniValue::YiniValue() : m_value(std::monostate{}) {}

    // Primitive type constructors
    YiniValue::YiniValue(bool value) : m_value(value) {}
    YiniValue::YiniValue(double value) : m_value(value) {}
    YiniValue::YiniValue(const std::string& value) : m_value(value) {}
    YiniValue::YiniValue(const char* value) : m_value(std::string(value)) {}

    // Constructors for recursive types
    YiniValue::YiniValue(YiniArray&& value) : m_value(std::make_unique<YiniArray>(std::move(value))) {}
    YiniValue::YiniValue(const YiniArray& value) : m_value(std::make_unique<YiniArray>(value)) {}
    YiniValue::YiniValue(YiniMap&& value) : m_value(std::make_unique<YiniMap>(std::move(value))) {}
    YiniValue::YiniValue(const YiniMap& value) : m_value(std::make_unique<YiniMap>(value)) {}
    YiniValue::YiniValue(DynaValue&& value) : m_value(std::make_unique<DynaValue>(std::move(value))) {}
    YiniValue::YiniValue(const DynaValue& value) : m_value(std::make_unique<DynaValue>(value)) {}

    // Destructor
    YiniValue::~YiniValue() = default;

    // Copy constructor
    YiniValue::YiniValue(const YiniValue& other)
    {
        std::visit([&](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                if (v) m_value = std::make_unique<YiniArray>(*v);
                else m_value = std::make_unique<YiniArray>();
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniMap>>) {
                if (v) m_value = std::make_unique<YiniMap>(*v);
                else m_value = std::make_unique<YiniMap>();
            } else if constexpr (std::is_same_v<T, std::unique_ptr<DynaValue>>) {
                if (v) m_value = std::make_unique<DynaValue>(*v);
                else m_value = std::make_unique<DynaValue>(YiniValue{});
            }
            else {
                m_value = v;
            }
        }, other.m_value);
    }

    // Copy assignment
    YiniValue& YiniValue::operator=(const YiniValue& other)
    {
        if (this != &other)
        {
            YiniValue temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    // Move constructor
    YiniValue::YiniValue(YiniValue&& other) noexcept = default;

    // Move assignment
    YiniValue& YiniValue::operator=(YiniValue&& other) noexcept = default;

} // namespace YINI