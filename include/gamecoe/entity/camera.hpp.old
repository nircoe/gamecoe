#pragma once

#include <gamecoe/entity/component.hpp>
#include <gamecoe/utils/shape.hpp>
#include <gamecoe/utils/frustum.hpp>
#include <gamecoe_config.hpp>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#if GAMECOE_HAS_UBO
#include <gamecoe/graphics/graphics_buffer.hpp>
#include <optional>
#endif

namespace gamecoe
{
    class Transform;
    
    class Camera : public Component<Camera>
    {
        float m_fov;
        float m_nearPlane;
        float m_farPlane;
        float m_orthographicHeight;

        bool m_perspective;

        mutable bool m_projectionCached;
        mutable glm::mat4 m_projectionMatrix;

        mutable bool m_frustumCached;
        mutable Frustum m_frustum;

#if GAMECOE_HAS_UBO
        std::optional<GraphicsBuffer> m_uniformBuffer;
#endif

        void calculateFrustum() const;

    public:
        static constexpr const char* TYPE_NAME = "Camera";

        Camera(GameObject &owner);
        Camera(const Camera &other) = delete;
        Camera &operator=(const Camera &other) = delete;
        Camera(Camera &&other) = delete;
        Camera &operator=(Camera &&other) = delete;

        virtual ~Camera() override { }

        virtual void initialize() override { }
        virtual void begin() override { }
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;

        void setFov(float fov);
        void setNearPlane(float nearPlane);
        void setFarPlane(float farPlane);
        void setPlanes(float nearPlane, float farPlane);

        void setPerspectiveMode();
        void setOrthographicMode(float orthographicSize);

        float fov() const;
        float nearPlane() const;
        float farPlane() const;
        // Returns a pair of floats - first the near plane and second the far plane
        std::pair<float, float> planes() const;

        bool perspective() const;
        bool orthographic() const;

        glm::mat4 viewMatrix() const;
        const glm::mat4 &projectionMatrix() const;

        const Frustum &frustum() const;

        // Returns if a 2D object is facing the Camera
        bool facing(const Transform &transform) const;
    };
} // namespace gamecoe
