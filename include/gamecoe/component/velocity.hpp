#pragma once

#include <glm/glm.hpp>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct velocity
        {
            glm::vec3 linear{0.0f};
            glm::vec3 angular{0.0f};
        };

        static_assert(std::is_standard_layout_v<velocity>);
    } // namespace components
} // namespace gamecoe
