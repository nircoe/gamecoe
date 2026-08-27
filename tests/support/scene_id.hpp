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

    inline std::string to_string(scene_id id)
    {
        return "TestScene" + std::to_string(static_cast<std::uint16_t>(id));
    }
} // namespace gamecoe
