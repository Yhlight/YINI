#include "Ast.h"
#include <variant>

namespace YINI
{
    std::unique_ptr<YiniValue> YiniValue::clone() const
    {
        auto new_value = std::make_unique<YiniValue>();
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<Array>>)
            {
                auto new_array = std::make_unique<Array>();
                new_array->reserve(arg->size());
                for (const auto& item : *arg)
                {
                    new_array->push_back(std::move(*item.clone()));
                }
                new_value->value = std::move(new_array);
            }
            else if constexpr (std::is_same_v<T, std::unique_ptr<Map>>)
            {
                auto new_map = std::make_unique<Map>();
                for (const auto& [key, val] : *arg)
                {
                    new_map->emplace(key, std::move(*val.clone()));
                }
                new_value->value = std::move(new_map);
            }
            else if constexpr (std::is_same_v<T, DynaWrapper>)
            {
                new_value->value = arg->clone();
            }
            else
            {
                new_value->value = arg;
            }
        }, value);
        return new_value;
    }
}