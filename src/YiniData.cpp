#include "YINI/YiniData.hpp"

namespace YINI
{
    // Helper for deep copying the variant
    YiniVariant deep_copy_variant(const YiniVariant& v) {
        return std::visit([](const auto& arg) -> YiniVariant {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<YiniArray>>) {
                if (!arg) return std::unique_ptr<YiniArray>(nullptr);
                auto new_arr = std::make_unique<YiniArray>();
                for (const auto& elem : arg->elements) {
                    new_arr->elements.push_back(elem); // relies on YiniValue's copy constructor
                }
                return new_arr;
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniMap>>) {
                if (!arg) return std::unique_ptr<YiniMap>(nullptr);
                auto new_map = std::make_unique<YiniMap>();
                for (const auto& [key, val] : arg->elements) {
                    new_map->elements[key] = val; // relies on YiniValue's copy constructor
                }
                return new_map;
            } else if constexpr (std::is_same_v<T, std::unique_ptr<YiniDynaValue>>) {
                if (!arg) return std::unique_ptr<YiniDynaValue>(nullptr);
                auto new_dyna = std::make_unique<YiniDynaValue>();
                new_dyna->value = arg->value; // relies on YiniValue's copy constructor
                return new_dyna;
            } else {
                return arg; // For non-pointer types, copy is fine
            }
        }, v);
    }

    YiniValue::YiniValue() = default;
    YiniValue::~YiniValue() = default;
    YiniValue::YiniValue(YiniValue&& other) noexcept = default;
    YiniValue& YiniValue::operator=(YiniValue&& other) noexcept = default;

    YiniValue::YiniValue(const YiniValue& other)
        : data(deep_copy_variant(other.data)) {}

    YiniValue& YiniValue::operator=(const YiniValue& other)
    {
        if (this != &other)
        {
            data = deep_copy_variant(other.data);
        }
        return *this;
    }
}