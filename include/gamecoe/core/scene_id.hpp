#pragma once

#include <cstdint>

namespace gamecoe
{
    // Fixed underlying type keeps this complete for storage, comparison, and pass-by-value with no enumerators needed.
    // gencoe generates the concrete enum body into the game project, extending namespace gamecoe.
    enum class scene_id : std::uint16_t;
} // namespace gamecoe
