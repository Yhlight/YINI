#include "DynaValue.h"

#include "YiniValue.h"

namespace YINI {
DynaValue::DynaValue(YiniValue value) : m_value(std::make_unique<YiniValue>(std::move(value))) {}

DynaValue::~DynaValue() = default;

DynaValue::DynaValue(const DynaValue& other) {
    if (other.m_value) {
        m_value = std::make_unique<YiniValue>(*other.m_value);
    } else {
        m_value = nullptr;
    }
}

DynaValue& DynaValue::operator=(const DynaValue& other) {
    if (this != &other) {
        if (other.m_value) {
            m_value = std::make_unique<YiniValue>(*other.m_value);
        } else {
            m_value = nullptr;
        }
    }
    return *this;
}

DynaValue::DynaValue(DynaValue&& other) noexcept = default;

DynaValue& DynaValue::operator=(DynaValue&& other) noexcept = default;

const YiniValue& DynaValue::get() const { return *m_value; }

void DynaValue::set(YiniValue value) { *m_value = std::move(value); }
}  // namespace YINI