#pragma once

#include <gamecoe/utils/shape.hpp>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct shape_collider
        {
            shape kind = shape::invalid;

            static shape_collider triangle() { return { shape::triangle }; }
            static shape_collider rectangle() { return { shape::rectangle }; }
            static shape_collider box() { return { shape::box }; }
            static shape_collider circle() { return { shape::circle }; }
            static shape_collider sphere() { return { shape::sphere }; }
        };

        static_assert(std::is_standard_layout_v<shape_collider>);
    } // namespace components
} // namespace gamecoe
