#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gamecoe
{
    const glm::vec3 &Transform::position() const
    {
        return m_position;
    }

    const glm::quat &Transform::rotation() const
    {
        return m_rotation;
    }

    glm::vec3 Transform::eulerRotation() const
    {
        return glm::eulerAngles(m_rotation);
    }

    const glm::vec3 &Transform::scale() const
    {
        return m_scale;
    }

    glm::mat4 Transform::translationMatrix() const
    {
        return glm::translate(s_identityMatrix, m_position);
    }

    glm::mat4 Transform::rotationMatrix() const
    {
        return glm::mat4_cast(m_rotation);
    }

    glm::mat4 Transform::scaleMatrix() const
    {
        return glm::scale(s_identityMatrix, m_scale);
    }

    glm::mat4 Transform::localMatrix() const
    {
        return translationMatrix() * rotationMatrix() * scaleMatrix();
    }

    glm::mat4 Transform::modelMatrix() const
    {
        GameObject *ownerParent = m_owner->parent();
        if(!ownerParent) return localMatrix();

        return ownerParent->transform().modelMatrix() * localMatrix();
    }

    void Transform::setPosition(const glm::vec3 &position)
    {
        m_position = position;
    }

    void Transform::setRotation(const glm::quat &rotation)
    {
        m_rotation = rotation;
    }

    void Transform::setRotation(const glm::vec3 &eulerAngles)
    {
        m_rotation = glm::quat(eulerAngles);
    }

    void Transform::setScale(const glm::vec3 &scale)
    {
        m_scale = scale;
    }

    void Transform::translate(const glm::vec3 &offset)
    {
        m_position += offset;
    }

    void Transform::rotate(const glm::vec3 &eulerOffset)
    {
        m_rotation = glm::quat(eulerOffset) * m_rotation;
    }

    void Transform::rotateAround(const glm::vec3 &axis, float angle)
    {
        m_rotation *= glm::angleAxis(angle, glm::normalize(axis));
    }

    glm::vec3 Transform::forward() const
    {
        return m_rotation * s_forwardVector;
    }

    glm::vec3 Transform::right() const
    {
        return m_rotation * s_rightVector;
    }

    glm::vec3 Transform::up() const
    {
        return m_rotation * s_upVector;
    }

    void Transform::lookAt(const glm::vec3 &target, const glm::vec3 &up)
    {
        glm::vec3 direction = glm::normalize(target - m_position);
        m_rotation = glm::quatLookAt(direction, up);
    }
} // namespace gamecoe
