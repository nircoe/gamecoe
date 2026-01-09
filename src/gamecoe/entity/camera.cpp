#include <gamecoe/entity/camera.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/core/window.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/consts.hpp>

namespace gamecoe
{
    Camera::Camera(GameObject &owner) : Component(owner), 
                                        m_fov(45.0f),
                                        m_nearPlane(0.1f),
                                        m_farPlane(100.0f),
                                        m_orthographicHeight(-1.0f),
                                        m_perspective(true),
                                        m_projectionCached(false)
    {
#if GAMECOE_HAS_UBO
        m_uniformBuffer.emplace(UniformBuffer(constcoe::CAMERA_UBO_BINDING_POINT));
#endif
    }
    
    void Camera::activate() 
    { 
        m_active = true;
    }

    void Camera::deactivate() 
    { 
        m_active = false;
    }

    void Camera::update()
    {
#if GAMECOE_HAS_UBO
        struct CameraUniformData
        {
            alignas(16) glm::mat4 m_view;
            alignas(16) glm::mat4 m_projection;
        } data;
        data.m_view = viewMatrix();
        data.m_projection = projectionMatrix();

        m_uniformBuffer->uploadData(&data, sizeof(data));
#endif
    }

    void Camera::setFov(float fov) 
    {
        if (fov <= 0.0f || 180.0f <= fov)
            detail::invalidArgument("Camera::setFov(): FOV must be between 0 and 180 degrees");

        m_fov = fov;
        m_projectionCached = false;
    }

    void Camera::setNearPlane(float nearPlane) 
    {
        if (nearPlane <= 0.0f)
            detail::invalidArgument("Camera::setNearPlane(): Near plane must be positive");
        if (nearPlane >= m_farPlane)
            detail::throwError("Camera::setNearPlane(): Far plane (" + std::to_string(m_farPlane) + 
                               ") must be greater than the near plane (" + std::to_string(nearPlane) + ")");

        m_nearPlane = nearPlane; 
        m_projectionCached = false;
    }

    void Camera::setFarPlane(float farPlane) 
    {
        if (farPlane <= 0.0f)
            detail::invalidArgument("Camera::setFarPlane(): Far plane must be positive");
        if (farPlane <= m_nearPlane)
            detail::throwError("Camera::setFarPlane(): Far plane (" + std::to_string(farPlane) + 
                               ") must be greater than the near plane (" + std::to_string(m_nearPlane) + ")");

        m_farPlane = farPlane;
        m_projectionCached = false;
    }

    void Camera::setPlanes(float nearPlane, float farPlane)
    {
        if (nearPlane <= 0.0f || farPlane <= 0.0f)
            detail::invalidArgument("Camera::setPlanes(): Both planes must be positive");
        if (nearPlane >= farPlane)
            detail::throwError("Camera::setPlanes(): Far plane must be greater than the near plane");

        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        m_projectionCached = false;
    }

    void Camera::setPerspectiveMode() 
    { 
        if (m_perspective) return;

        m_orthographicHeight = -1.0f;
        m_projectionCached = false;
        m_perspective = true;
    }

    void Camera::setOrthographicMode(float orthographicSize) 
    { 
        if (!m_perspective) return;

        if (orthographicSize <= 0.0f) 
            detail::invalidArgument("Camera::setOrthographicMode(): Orthographic size must be positive");

        m_orthographicHeight = orthographicSize;
        m_projectionCached = false;
        m_perspective = false;
    }

    float Camera::fov() const { return m_fov; }

    float Camera::nearPlane() const { return m_nearPlane; }

    float Camera::farPlane() const { return m_farPlane; }

    std::pair<float, float> Camera::planes() const { return { m_nearPlane, m_farPlane }; }

    bool Camera::perspective() const { return m_perspective; }

    bool Camera::orthographic() const { return !m_perspective; }

    glm::mat4 Camera::viewMatrix() const 
    { 
        const auto &transform = m_owner.transform();
        const auto &position = transform.position();

        return glm::lookAt(position, position + transform.forward(), transform.up());
    }

    const glm::mat4 &Camera::projectionMatrix() const 
    { 
        if (!m_projectionCached)
        {
            float aspectRatio = m_owner.game().mainWindow().aspectRatio();
            if (m_perspective)
                m_projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
            else
            {
                float halfHeight = m_orthographicHeight / 2.0f;
                float halfWidth = halfHeight * aspectRatio;
                m_projectionMatrix = glm::ortho(-halfWidth, halfWidth, 
                                                -halfHeight, halfHeight,
                                                m_nearPlane, m_farPlane);
            }
            m_projectionCached = true;
        }

        return m_projectionMatrix;
    }
} // namespace gamecoe
