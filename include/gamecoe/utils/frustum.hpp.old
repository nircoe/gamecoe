#pragma once

#include <gamecoe/utils/shape.hpp>
#include <glm/glm.hpp>

namespace gamecoe
{
    class Transform;

    struct Frustum
    {
        struct Plane
        {
            // The normal vector to the plane
            glm::vec3 m_normal;
            // A point on the plane
            glm::vec3 m_point;
        };

        Plane m_topFace;
        Plane m_bottomFace;
        Plane m_rightFace;
        Plane m_leftFace;
        Plane m_nearFace;
        Plane m_farFace;

        bool contains(const Transform &transform, Shape shape) const;
    };
} // namespace gamecoe
