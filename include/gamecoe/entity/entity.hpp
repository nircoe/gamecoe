#pragma once

#include <cstdint>
#include <cassert>
#include <limits>
#include <compare>

namespace gamecoe
{
    struct entity
    {
        static constexpr std::uint32_t INVALID = std::numeric_limits<std::uint32_t>::max();
        static constexpr std::uint8_t ID_BITS = 20;
        static constexpr std::uint8_t GEN_BITS = 12;
        static constexpr std::uint32_t ID_MASK = (1U << ID_BITS) - 1;
        static constexpr std::uint32_t GEN_MASK = (1U << GEN_BITS) - 1;
        static constexpr std::uint32_t MAX_ENTITIES = ID_MASK - 1;  // 1,048,574
        static constexpr std::uint16_t MAX_GENERATIONS = GEN_MASK;  // 4,095

        static entity create(std::uint32_t id, std::uint16_t generation)
        {
            assert(id <= MAX_ENTITIES && "entity::create(): entity id exceeds maximum");
            assert(generation <= MAX_GENERATIONS && "entity::create(): entity generation exceeds maximum");
            // Bit layout (id-major ensures default operator<=> orders by id first, generation second)
            // [31..................12][11..............0] 
            // [       20-bit id      ][12-bit generation]
            return entity((id << GEN_BITS) | generation);
        }

        auto operator<=>(const entity &other) const = default;
        bool operator==(const entity &other) const = default;

        bool valid() const { return m_value < INVALID; }

        std::uint32_t id() const { return (m_value >> GEN_BITS) & ID_MASK; }
        std::uint16_t generation() const { return m_value & GEN_MASK; }
    
    private:
        std::uint32_t m_value;

        explicit entity(std::uint32_t value = INVALID) : m_value(value) { }
    };
} // namespace gamecoe
