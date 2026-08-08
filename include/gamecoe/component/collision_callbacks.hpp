#pragma once

#include <type_traits>

namespace gamecoe
{
    struct entity;
    class entities;

    namespace components
    {
        // Takes two entities& even though the engine only has one shared registry today -
        // kept dual-parameter shaped for forward-compatibility,
        // both are the same reference passed twice at the call site under the current single-registry model.
        // Revisit collapsing to a single entities& parameter if a second registry never materializes.
        using collision_callback = void(*)(entity self, entities& self_registry, entity other, entities& other_registry);

        struct collision_callbacks
        {
            collision_callback on_begin = nullptr;
            collision_callback on_continuous = nullptr;
            collision_callback on_end = nullptr;
        };

        static_assert(std::is_standard_layout_v<collision_callbacks>);
    } // namespace components
} // namespace gamecoe
