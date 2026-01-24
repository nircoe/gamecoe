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

        mutable glm::mat4 m_cachedModelMatrix{1.0f};
        mutable glm::mat4 m_cachedInverseModelMatrix{1.0f};
        mutable bool m_modelChanged = true;
        mutable bool m_cachedModelValid = false;

        void invalidateCachedModel() const;

        static constexpr glm::mat4 s_identityMatrix{1.0f};
        static constexpr glm::vec3 s_forwardVector{0.0f, 0.0f, -1.0f};
        static constexpr glm::vec3 s_rightVector{1.0f, 0.0f, 0.0f};
        static constexpr glm::vec3 s_upVector{0.0f, 1.0f, 0.0f};
    
    public:
        static constexpr const char* TYPE_NAME = "Transform";

        Transform(GameObject &owner) : Component<Transform>(owner), m_position(0.0f), m_rotation(1.0f, 0.0f, 0.0f, 0.0f), m_scale(1.0f) { }
        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;
        Transform(Transform&&) = delete;
        Transform& operator=(Transform&&) = delete;
        virtual ~Transform() override = default;

        // initialize empty for now, maybe will be useful in the future
        virtual void initialize() override { }
        // begin empty for now, maybe will be useful in the future
        virtual void begin() override { }
        virtual void activate() override { m_active = true; }
        virtual void deactivate() override { m_active = false; }
        // other related components will update the state of Transform directly
        virtual void update() override { }

        // Gets the local space position
        const glm::vec3 &position() const;
        // Gets the local space 2D position
        glm::vec2 position2D() const;
        // Gets the local space quaternion rotation
        const glm::quat &rotation() const;
        // Gets the local space euler angles rotation
        glm::vec3 eulerRotation() const;
        // Gets the local space euler angles 2D rotation
        glm::vec2 eulerRotation2D() const;
        // Gets the local space scale
        const glm::vec3 &scale() const;
        // Gets the local space 2D scale
        glm::vec2 scale2D() const;

        // Gets the world space position
        glm::vec3 worldPosition() const;
        // Gets the world space 2D position
        glm::vec2 worldPosition2D() const;
        // Gets the world space quaternion rotation
        glm::quat worldRotation() const;
        // Gets the world space euler angles rotation
        glm::vec3 worldEulerRotation() const;
        // Gets the world space euler angles 2D rotation
        glm::vec2 worldEulerRotation2D() const;
        // Gets the world space scale
        glm::vec3 worldScale() const;
        // Gets the world space 2D scale
        glm::vec2 worldScale2D() const;

        // Returns the local translation matrix
        glm::mat4 translationMatrix() const;
        // Returns the local rotation matrix
        glm::mat4 rotationMatrix() const;
        // Returns the local scale matrix
        glm::mat4 scaleMatrix() const;
        // Returns the local transformation matrix
        glm::mat4 localMatrix() const;
        // Returns world transformation matrix
        const glm::mat4 &modelMatrix() const;
        // Returns the inversed world transformation matrix
        const glm::mat4 &inverseModelMatrix() const;

        // Sets the local space position
        void setPosition(const glm::vec3 &position);
        // Sets the local space 2D position
        void setPosition(const glm::vec2 &position);
        // Sets the local space quaternion rotation
        void setRotation(const glm::quat &rotation);
        // Sets the local space euler angles rotation
        void setRotation(const glm::vec3 &eulerAngles);
        // Sets the local space euler angles 2D rotation
        void setRotation(const glm::vec2 &eulerAngles);
        // Sets the local space scale
        void setScale(const glm::vec3 &scale);
        // Sets the local space 2D scale
        void setScale(const glm::vec2 &scale);

        // Sets the world space position
        void setWorldPosition(const glm::vec3 &position);
        // Sets the world space 2D position
        void setWorldPosition(const glm::vec2 &position);
        // Sets the world space quaternion rotation
        void setWorldRotation(const glm::quat &rotation);
        // Sets the world space euler angles rotation
        void setWorldRotation(const glm::vec3 &eulerAngles);
        // Sets the world space euler angles 2D rotation
        void setWorldRotation(const glm::vec2 &eulerAngles);
        // Sets the world space scale
        void setWorldScale(const glm::vec3 &scale);
        // Sets the world space 2D scale
        void setWorldScale(const glm::vec2 &scale);

        // Translates in local space
        void translate(const glm::vec3 &offset);
        // Translates in local space (2D)
        void translate(const glm::vec2 &offset);
        // Rotates in local space
        void rotate(const glm::vec3 &eulerOffset);
        // Rotates in local space (2D)
        void rotate(const glm::vec2 &eulerOffset);
        // Rotates around axis in local space
        void rotateAround(const glm::vec3 &axis, float angle);
        // Rotates around the z-axis in local space (2D)
        void rotateAround(float angle);

        // Returns the normalized local forward direction vector
        glm::vec3 forward() const;
        // Returns the normalized local right direction vector
        glm::vec3 right() const;
        // Returns the normalized local up direction vector
        glm::vec3 up() const;

        // Returns the normalized world forward direction vector
        glm::vec3 worldForward() const;
        // Returns the normalized world right direction vector  
        glm::vec3 worldRight() const;
        // Returns the normalized world up direction vector
        glm::vec3 worldUp() const;

        // Rotates Transform to look at target (world space coordinates)
        void lookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));
        // Rotates Transform to look at target (world space 2D coordinates)
        void lookAt(const glm::vec2 &target);

        // Returns if the transform moved in the current frame
        bool modelChanged() const;
        // Clears the modelChanged flag
        void clearModelChanged() const;
    };
} // namespace gamecoe