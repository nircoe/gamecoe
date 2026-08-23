#pragma once

#include <cstdint>
#include <string>

namespace gamecoe
{
    // Forward declaration to gencoe's generated implementation.
    // If you're not using gencoe, define the enum and to_string() yourself.

    enum class scene_id : std::uint16_t;

    [[nodiscard]] std::string to_string(scene_id id);
} // namespace gamecoe
