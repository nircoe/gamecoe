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
            gamecoe::color color;

            static shape_renderer triangle(const gamecoe::color &color_) { return { shape::triangle, color_ }; }
            static shape_renderer rectangle(const gamecoe::color &color_) { return { shape::rectangle, color_ }; }
            static shape_renderer box(const gamecoe::color &color_) { return { shape::box, color_ }; }
            static shape_renderer circle(const gamecoe::color &color_) { return { shape::circle, color_ }; }
            static shape_renderer sphere(const gamecoe::color &color_) { return { shape::sphere, color_ }; }
        };

        static_assert(std::is_standard_layout_v<shape_renderer>);
    } // namespace components
} // namespace gamecoe
