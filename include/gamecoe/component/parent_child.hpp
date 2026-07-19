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
            entity handle = entity::invalid();

            bool has_parent() const { return handle != entity::invalid(); }
        };

        struct children
        {
            std::vector<entity> handles;

            bool has_children() const { return !handles.empty(); }
        };

        static_assert(std::is_standard_layout_v<parent>);
        static_assert(std::is_standard_layout_v<children>);
    } // namespace components
} // namespace gamecoe
