#include <gamecoe/utils/frustum.hpp>
#include <gamecoe/entity/transform.hpp>

namespace gamecoe
{
    bool Frustum::contains(const Transform &transform, Shape shape) const
    {
        // Conservative frustum culling using bounding spheres for all shapes
        // False positives (rendering slightly off screen objects) are acceptable

        auto center = transform.worldPosition();
        auto scale = transform.worldScale();

        float radius;
        switch (shape)
        {
        case Shape::Triangle:
        case Shape::Rectangle:
        case Shape::Circle:
            radius = glm::max(scale.x, scale.y);
            break;
        case Shape::Box:
            radius = glm::length(scale) * 0.5f;
            break;
        case Shape::Sphere:
        default:
            radius = glm::max(glm::max(scale.x, scale.y), scale.z);
            break;
        }

        Plane planes[6] = { m_topFace, m_bottomFace, m_rightFace, m_leftFace, m_nearFace, m_farFace };

        for(auto &plane : planes)
        {
            float distance = glm::dot(plane.m_normal, center - plane.m_point);
            if (distance < -radius)
                return false;
        }

        return true;
    }
} // namespace gamecoe
