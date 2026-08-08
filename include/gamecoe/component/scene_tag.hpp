#pragma once

#include <gamecoe/core/scene_id.hpp>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct scene_tag
        {
            scene_id id{};
        };

        static_assert(std::is_standard_layout_v<scene_tag>);
    } // namespace components
} // namespace gamecoe
