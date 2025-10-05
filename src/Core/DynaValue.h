#pragma once

#pragma once

#include <memory>

#include "YiniValue.h"

namespace YINI {
class DynaValue {
public:
    DynaValue(YiniValue value);
    ~DynaValue();
    DynaValue(const DynaValue& other);
    DynaValue& operator=(const DynaValue& other);
    DynaValue(DynaValue&& other) noexcept;
    DynaValue& operator=(DynaValue&& other) noexcept;

    const YiniValue& get() const;
    void set(YiniValue value);

    std::unique_ptr<YiniValue> m_value;
};
}  // namespace YINI