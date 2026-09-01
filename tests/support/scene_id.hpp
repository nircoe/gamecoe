#pragma once

#include <gamecoe/core/scene_id.hpp>
#include <cstdint>
#include <string>

namespace gamecoe
{
    enum class scene_id : std::uint16_t
    {
        TestScene1 = 1,
        TestScene2 = 2,
    };

    std::string to_string(scene_id id);
} // namespace gamecoe
