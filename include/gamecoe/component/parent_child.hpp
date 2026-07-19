#pragma once

#include <gamecoe/entity/entity.hpp>
#include <vector>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct parent
        {
            entity parent = entity::invalid();

            bool has_parent() const { return parent != entity::invalid(); }
        };

        struct children
        {
            std::vector<entity> children;

            bool has_children() const { return !children.empty(); }
        };

        static_assert(std::is_standard_layout_v<parent>);
        static_assert(std::is_standard_layout_v<children>);
    } // namespace components
} // namespace gamecoe
