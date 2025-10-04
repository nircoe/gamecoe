#pragma once

#include <gamecoe/entity/component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

namespace gamecoe
{
    class Transform : public Component<Transform>
    {
        glm::vec3 m_position;
        glm::quat m_rotation;
        glm::vec3 m_scale;

        static constexpr glm::mat4 s_identityMatrix{1.0f};
        static constexpr glm::vec3 s_forwardVector{0.0f, 0.0f, -1.0f};
        static constexpr glm::vec3 s_rightVector{1.0f, 0.0f, 0.0f};
        static constexpr glm::vec3 s_upVector{0.0f, 1.0f, 0.0f};
    
    public:
        static constexpr const char* TYPE_NAME = "Transform";

        Transform(GameObject *parent) : Component<Transform>(parent), m_position(0.0f), m_rotation(1.0f, 0.0f, 0.0f, 0.0f), m_scale(1.0f) { };
        virtual ~Transform() override = default; // default is fine?

        // initialize empty for now, maybe will be useful in the future
        virtual void initialize() override { };
        // begin empty for now, maybe will be useful in the future
        virtual void begin() override { };
        // can't activate/deactivate Transform - does nothing
        virtual void activate() override { };
        // can't activate/deactivate Transform - does nothing
        virtual void deactivate() override { };
        // other related components will update the state of Transform directly
        virtual void update() override { };

        // Gets a float vector 3 for the position in the x,y,z axis
        const glm::vec3 &position() const;
        // Gets a float quaternion for the rotation
        const glm::quat &rotation() const;
        // Gets a float vector 3 for the rotation Euler angles
        glm::vec3 eulerRotation() const;
        // Gets a float vector 3 for the scale multiplier in the x,y,z axis
        const glm::vec3 &scale() const;

        glm::mat4 translationMatrix() const;
        glm::mat4 rotationMatrix() const;
        glm::mat4 scaleMatrix() const;
        glm::mat4 localMatrix() const;
        glm::mat4 modelMatrix() const;

        void setPosition(const glm::vec3 &position);
        void setRotation(const glm::quat &rotation);
        void setRotation(const glm::vec3 &eulerAngles);
        void setScale(const glm::vec3 &scale);

        void translate(const glm::vec3 &offset);
        // Rotates the Transform in local space
        void rotate(const glm::vec3 &eulerOffset);
        // Rotates the Transform in world space
        void rotateAround(const glm::vec3 &axis, float angle);

        glm::vec3 forward() const;
        glm::vec3 right() const;
        glm::vec3 up() const;

        void lookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));
    };
} // namespace gamecoe