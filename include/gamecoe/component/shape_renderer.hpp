#pragma once

#include <gamecoe/utils/shape.hpp>
#include <colorcoe.hpp>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct shape_renderer
        {
            shape kind = shape::invalid;
            Color color;

            static shape_renderer triangle(const Color &color) { return { shape::triangle, color }; }
            static shape_renderer rectangle(const Color &color) { return { shape::rectangle, color }; }
            static shape_renderer box(const Color &color) { return { shape::box, color }; }
            static shape_renderer circle(const Color &color) { return { shape::circle, color }; }
            static shape_renderer sphere(const Color &color) { return { shape::sphere, color }; }
        };

        static_assert(std::is_standard_layout_v<shape_renderer>);
    } // namespace components
} // namespace gamecoe
