#pragma once

#include <gamecoe/entity/transform.hpp>
#include <gamecoe/utils/shape.hpp>
#include <utility>
#include <cstdint>

namespace gamecoe
{
    namespace collision
    {
        // Collision detection for ShapeCollider/ShapeCollider
        bool detect(const Transform &t1, Shape s1, const Transform &t2, Shape s2);
        // TODO: add overload detect methods for different Colliders
    } // namespace collision
} // namespace gamecoe
