#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gamecoe
{
    void Transform::invalidateCachedModel() const
    {
        m_modelChanged = true;

        auto &children = m_owner.children();
        for (auto &child : children)
            child.get().transform().invalidateCachedModel();
    }

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

    glm::vec3 Transform::worldPosition() const
    {
        const auto &model = modelMatrix();
        return glm::vec3(model[3]);
    }

    glm::quat Transform::worldRotation() const
    {
        const auto &model = modelMatrix();

        auto wScale = worldScale();
        auto rotationMatrix = glm::mat3(model);
        rotationMatrix[0] /= wScale.x;
        rotationMatrix[1] /= wScale.y;
        rotationMatrix[2] /= wScale.z;

        return glm::quat_cast(rotationMatrix);
    }

    glm::vec3 Transform::worldEulerRotation() const
    {
        return glm::eulerAngles(worldRotation());
    }

    glm::vec3 Transform::worldScale() const
    {
        const auto &model = modelMatrix();

        return glm::vec3(
            glm::length(model[0]),
            glm::length(model[1]),
            glm::length(model[2])
        );
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

    const glm::mat4 &Transform::modelMatrix() const
    {
        if (!m_modelChanged) return m_cachedModelMatrix;

        glm::mat4 local = localMatrix();

        m_cachedModelMatrix = !m_owner.hasParent() ? local : m_owner.parent()->get().transform().modelMatrix() * local;
        m_cachedInverseModelMatrix = glm::inverse(m_cachedModelMatrix);
        m_modelChanged = false;

        return m_cachedModelMatrix;
    }

    const glm::mat4 &Transform::inverseModelMatrix() const
    {
        modelMatrix();
        return m_cachedInverseModelMatrix;
    }

    void Transform::setPosition(const glm::vec3 &position)
    {
        m_position = position;
        invalidateCachedModel();
    }

    void Transform::setRotation(const glm::quat &rotation)
    {
        m_rotation = rotation;
        invalidateCachedModel();
    }

    void Transform::setRotation(const glm::vec3 &eulerAngles)
    {
        m_rotation = glm::quat(eulerAngles);
        invalidateCachedModel();
    }

    void Transform::setScale(const glm::vec3 &scale)
    {
        m_scale = scale;
        invalidateCachedModel();
    }

    void Transform::setWorldPosition(const glm::vec3 &position)
    {
        if (!m_owner.hasParent()) return setPosition(position);

        const auto &parentModel = m_owner.parent()->get().transform().modelMatrix();
        auto localPos = glm::inverse(parentModel) * glm::vec4(position, 1.0f);
        
        setPosition(glm::vec3(localPos));
    }

    void Transform::setWorldRotation(const glm::quat &rotation)
    {
        if (!m_owner.hasParent()) return setRotation(rotation);

        auto parentWorldRotation = m_owner.parent()->get().transform().worldRotation();
        auto localRotation = glm::inverse(parentWorldRotation) * rotation;
        
        setRotation(localRotation);
    }

    void Transform::setWorldRotation(const glm::vec3 &eulerAngles)
    {
        setWorldRotation(glm::quat(eulerAngles));
    }

    void Transform::setWorldScale(const glm::vec3 &scale)
    {
        if (!m_owner.hasParent()) return setScale(scale);

        auto parentWorldScale = m_owner.parent()->get().transform().worldScale();
        auto localScale = scale / parentWorldScale;

        setScale(localScale);
    }

    void Transform::translate(const glm::vec3 &offset)
    {
        m_position += offset;
        invalidateCachedModel();
    }

    void Transform::rotate(const glm::vec3 &eulerOffset)
    {
        m_rotation = glm::quat(eulerOffset) * m_rotation;
        invalidateCachedModel();
    }

    void Transform::rotateAround(const glm::vec3 &axis, float angle)
    {
        m_rotation *= glm::angleAxis(angle, glm::normalize(axis));
        invalidateCachedModel();
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

    glm::vec3 Transform::worldForward() const
    {
        return worldRotation() * s_forwardVector;
    }

    glm::vec3 Transform::worldRight() const
    {
        return worldRotation() * s_rightVector;
    }

    glm::vec3 Transform::worldUp() const
    {
        return worldRotation() * s_upVector;
    }

    void Transform::lookAt(const glm::vec3 &target, const glm::vec3 &up)
    {
        glm::vec3 direction = glm::normalize(target - worldPosition());
        setWorldRotation(glm::quatLookAt(direction, up));
    }
} // namespace gamecoe
