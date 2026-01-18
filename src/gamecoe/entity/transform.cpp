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

    glm::vec2 Transform::position2D() const
    {
        return { m_position.x, m_position.y };
    }

    const glm::quat &Transform::rotation() const
    {
        return m_rotation;
    }

    glm::vec3 Transform::eulerRotation() const
    {
        return glm::eulerAngles(m_rotation);
    }

    glm::vec2 Transform::eulerRotation2D() const
    {
        auto rotation = eulerRotation();
        return { rotation.x, rotation.y };
    }

    const glm::vec3 &Transform::scale() const
    {
        return m_scale;
    }

    glm::vec2 Transform::scale2D() const
    {
        return { m_scale.x, m_scale.y };
    }

    glm::vec3 Transform::worldPosition() const
    {
        const auto &model = modelMatrix();
        return glm::vec3(model[3]);
    }

    glm::vec2 Transform::worldPosition2D() const
    {
        auto wPos = worldPosition();
        return { wPos.x, wPos.y };
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

    glm::vec2 Transform::worldEulerRotation2D() const
    {
        auto rotation = worldEulerRotation();
        return { rotation.x, rotation.y };
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

    glm::vec2 Transform::worldScale2D() const
    {
        auto scale = worldScale();
        return { scale.x, scale.y };
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

    void Transform::setPosition(const glm::vec2 &position)
    {
        setPosition({ position.x, position.y, 0.0f });
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

    void Transform::setRotation(const glm::vec2 &eulerAngles)
    {
        setRotation({ eulerAngles.x, eulerAngles.y, 0.0f });
    }

    void Transform::setScale(const glm::vec3 &scale)
    {
        m_scale = scale;
        invalidateCachedModel();
    }

    void Transform::setScale(const glm::vec2 &scale)
    {
        setScale({ scale.x, scale.y, 1.0f });
    }

    void Transform::setWorldPosition(const glm::vec3 &position)
    {
        if (!m_owner.hasParent()) return setPosition(position);

        const auto &parentModel = m_owner.parent()->get().transform().modelMatrix();
        auto localPos = glm::inverse(parentModel) * glm::vec4(position, 1.0f);
        
        setPosition({ localPos.x, localPos.y, localPos.z });
    }

    void Transform::setWorldPosition(const glm::vec2 &position)
    {
        setWorldPosition({ position.x, position.y, 0.0f });
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

    void Transform::setWorldRotation(const glm::vec2 &eulerAngles)
    {
        setWorldRotation({ eulerAngles.x, eulerAngles.y, 0.0f });
    }

    void Transform::setWorldScale(const glm::vec3 &scale)
    {
        if (!m_owner.hasParent()) return setScale(scale);

        auto parentWorldScale = m_owner.parent()->get().transform().worldScale();
        auto localScale = scale / parentWorldScale;

        setScale(localScale);
    }

    void Transform::setWorldScale(const glm::vec2 &scale)
    {
        setWorldScale({ scale.x, scale.y, 1.0f });
    }

    void Transform::translate(const glm::vec3 &offset)
    {
        m_position += offset;
        invalidateCachedModel();
    }

    void Transform::translate(const glm::vec2 &offset)
    {
        translate({ offset.x, offset.y, 0.0f });
    }

    void Transform::rotate(const glm::vec3 &eulerOffset)
    {
        m_rotation = glm::quat(eulerOffset) * m_rotation;
        invalidateCachedModel();
    }

    void Transform::rotate(const glm::vec2 &eulerOffset)
    {
        rotate({ eulerOffset.x, eulerOffset.y, 0.0f });
    }

    void Transform::rotateAround(const glm::vec3 &axis, float angle)
    {
        m_rotation *= glm::angleAxis(angle, glm::normalize(axis));
        invalidateCachedModel();
    }

    void Transform::rotateAround(float angle)
    {
        rotateAround({ 0.0f, 0.0f, 1.0f }, angle);
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
        auto direction = glm::normalize(target - worldPosition());
        setWorldRotation(glm::quatLookAt(direction, up));
    }

    void Transform::lookAt(const glm::vec2 &target)
    {
        lookAt({ target.x, target.y, 0.0f });
    }
} // namespace gamecoe
