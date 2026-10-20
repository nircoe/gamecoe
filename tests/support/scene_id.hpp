#pragma once

#include <gamecoe/core/scene_id.hpp>
#include <cstdint>
#include <string>

namespace gamecoe
{
    enum class scene_id : std::uint16_t
    {
        test_scene1 = 1,
        test_scene2 = 2,
        test_scene3 = 3,
    };

    std::string to_string(scene_id id);
} // namespace gamecoe
