/**
 * @file DynaValue.h
 * @brief Defines the DynaValue class for creating dynamically updatable YINI values.
 * @ingroup Core
 */
#pragma once

#include "YiniValue.h"
#include <memory>

namespace YINI
{
    /**
     * @class DynaValue
     * @brief Represents a YINI value that can be modified at runtime.
     * @ingroup Core
     *
     * @details The `DynaValue` is a wrapper around a `YiniValue` that signifies
     * that the enclosed value can be updated dynamically. When a `DynaValue` is
     * modified, the change is tracked and can be persisted to a `.ymeta` metadata
     * file, allowing the state to be preserved across sessions.
     *
     * The special member functions are defined in the `.cpp` file because the
     * `std::unique_ptr` member points to an incomplete type (`YiniValue`) at this stage.
     */
    class DynaValue
    {
    public:
        /// @name Constructors and Destructor
        /// @{

        /**
         * @brief Constructs a DynaValue, taking ownership of the provided YiniValue.
         * @param value The initial `YiniValue` to be managed dynamically.
         */
        DynaValue(YiniValue value);

        /// @brief Destructor.
        ~DynaValue();
        /// @brief Copy constructor.
        DynaValue(const DynaValue& other);
        /// @brief Copy assignment operator.
        DynaValue& operator=(const DynaValue& other);
        /// @brief Move constructor.
        DynaValue(DynaValue&& other) noexcept;
        /// @brief Move assignment operator.
        DynaValue& operator=(DynaValue&& other) noexcept;

        /// @}

        /// @name Value Access and Modification
        /// @{

        /**
         * @brief Gets a constant reference to the underlying YiniValue.
         * @return A `const YiniValue&` to the managed value.
         */
        const YiniValue& get() const;

        /**
         * @brief Sets a new YiniValue, replacing the existing one.
         * @param value The new `YiniValue` to be managed.
         */
        void set(YiniValue value);

        /// @}

        /**
         * @brief A unique pointer to the heap-allocated YiniValue.
         *
         * @details This allows the `DynaValue` to be included recursively within
         * a `YiniValue` variant and manages the lifetime of the underlying value.
         */
        std::unique_ptr<YiniValue> m_value;
    };
}