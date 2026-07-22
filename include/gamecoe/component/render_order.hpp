#pragma once

#include <cstdint>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct render_order
        {
            std::int8_t layer = 0;
        };

        static_assert(std::is_standard_layout_v<render_order>);
    } // namespace components
} // namespace gamecoe
