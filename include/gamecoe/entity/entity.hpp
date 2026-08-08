#pragma once

#include <cstdint>
#include <gamecoe/utils/error_handler.hpp>
#include <limits>
#include <compare>

namespace gamecoe
{
    struct entity
    {
        // 20-bit id, 12-bit generation: sized for object-pooling workloads (roughly 1M entities,
        // ~4K generation-recycles per id) while keeping the whole handle at 4 bytes register-friendly.
        // MAX_ENTITIES and MAX_GENERATIONS reserve the top value to avoid sentinel collision (see invalid()).
        // Future: consider 32-bit id and separated 16-bit generation, 
        // or 64-bit handles for larger games requiring more ids and generations (with custom separation within the 64-bits).
        static constexpr std::uint8_t ID_BITS = 20;
        static constexpr std::uint8_t GEN_BITS = 12;
        static constexpr std::uint32_t ID_MASK = (1U << ID_BITS) - 1;
        static constexpr std::uint32_t GEN_MASK = (1U << GEN_BITS) - 1;
        static constexpr std::uint32_t MAX_ENTITIES = ID_MASK - 1;  // 1,048,574
        static constexpr std::uint16_t MAX_GENERATIONS = GEN_MASK - 1;  // 4,094

        // All-1s bit pattern, this sentinel is why MAX_ENTITIES and MAX_GENERATIONS reserve the top value.
        static constexpr entity invalid() noexcept { return entity{std::numeric_limits<std::uint32_t>::max()}; }

        auto operator<=>(const entity &other) const noexcept = default;
        bool operator==(const entity &other) const noexcept = default;

        std::uint32_t id() const noexcept { return m_handle >> GEN_BITS; }
        std::uint16_t generation() const noexcept { return m_handle & GEN_MASK; }

        static entity create(std::uint32_t id, std::uint16_t generation)
        {
            GAMECOE_ASSERT_LOG(id <= MAX_ENTITIES, "entity::create(): entity id exceeds maximum");
            GAMECOE_ASSERT_LOG(generation <= MAX_GENERATIONS, "entity::create(): entity generation exceeds maximum");
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
