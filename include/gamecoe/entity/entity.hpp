#pragma once

#include <cstdint>
#include <cassert>
#include <limits>
#include <compare>

namespace gamecoe
{
    struct entity
    {
        static constexpr std::uint8_t ID_BITS = 20;
        static constexpr std::uint8_t GEN_BITS = 12;
        static constexpr std::uint32_t ID_MASK = (1U << ID_BITS) - 1;
        static constexpr std::uint32_t GEN_MASK = (1U << GEN_BITS) - 1;
        static constexpr std::uint32_t MAX_ENTITIES = ID_MASK - 1;  // 1,048,574
        static constexpr std::uint16_t MAX_GENERATIONS = GEN_MASK - 1;  // 4,094

        static constexpr entity invalid() noexcept { return entity{std::numeric_limits<std::uint32_t>::max()}; }

        auto operator<=>(const entity &other) const noexcept = default;
        bool operator==(const entity &other) const noexcept = default;

        std::uint32_t id() const noexcept { return m_handle >> GEN_BITS; }
        std::uint16_t generation() const noexcept { return m_handle & GEN_MASK; }

        static entity create(std::uint32_t id, std::uint16_t generation)
        {
            assert(id <= MAX_ENTITIES && "entity::create(): entity id exceeds maximum");
            assert(generation <= MAX_GENERATIONS && "entity::create(): entity generation exceeds maximum");
            // Bit layout (id-major ensures default operator<=> orders by id first, generation second)
            // [31..................12][11..............0] 
            // [       20-bit id      ][12-bit generation]
            return entity((id << GEN_BITS) | generation);
        }
    
    private:
        std::uint32_t m_handle;

        explicit constexpr entity(std::uint32_t value) noexcept : m_handle(value) { }
    };
} // namespace gamecoe
