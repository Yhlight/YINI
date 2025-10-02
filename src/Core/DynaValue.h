#pragma once

#include <any>

namespace YINI
{
    class DynaValue
    {
    public:
        DynaValue(std::any value) : m_value(std::move(value)) {}

        const std::any& get() const { return m_value; }
        void set(std::any value) { m_value = std::move(value); }

    private:
        std::any m_value;
    };
}