#pragma once

#include <cstdint>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct collision_layer
        {
            std::int8_t layer = 0;
        };

        static_assert(std::is_standard_layout_v<collision_layer>);
    } // namespace components
} // namespace gamecoe
