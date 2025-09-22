#include "AST.h"

namespace YINI
{
    // Implementation of equality operators for deep comparison of ASTs.

    bool operator==(const Value& lhs, const Value& rhs)
    {
        if (lhs.data.index() != rhs.data.index())
        {
            return false;
        }

        return std::visit(
            [&](auto&& lhs_arg) -> bool {
                auto&& rhs_arg = std::get<std::decay_t<decltype(lhs_arg)>>(rhs.data);
                using T = std::decay_t<decltype(lhs_arg)>;

                if constexpr (std::is_same_v<T, Array>)
                {
                    if (lhs_arg.size() != rhs_arg.size()) return false;
                    for (size_t i = 0; i < lhs_arg.size(); ++i)
                    {
                        if (!(*lhs_arg[i] == *rhs_arg[i])) return false;
                    }
                    return true;
                }
                else if constexpr (std::is_same_v<T, Map>)
                {
                    if (lhs_arg.size() != rhs_arg.size()) return false;
                    return std::equal(lhs_arg.begin(), lhs_arg.end(), rhs_arg.begin(),
                        [](const auto& a, const auto& b){
                            return a.first == b.first && *a.second == *b.second;
                        });
                }
                else
                {
                    // For primitive types and structs with operator==
                    return lhs_arg == rhs_arg;
                }
            },
            lhs.data
        );
    }
}
