#pragma once

#include <cstdint>
#include <string>

namespace gamecoe
{
    // Fixed underlying type keeps this complete for storage, comparison, and pass-by-value with no enumerators needed.
    // gencoe generates the concrete enum body into the game project, extending namespace gamecoe.
    enum class scene_id : std::uint16_t;

    // gencoe generates the definition into the game project (a lookup over the enumerator names it
    // generates), same forward-declare-here/fill-in-there split as scene_id itself. A gamedev NOT
    // using gencoe must define scene_id's enumerators and this function themselves, or any code
    // calling gamecoe::game (which calls to_string internally) won't link.
    [[nodiscard]] std::string to_string(scene_id id);
} // namespace gamecoe
