#include <gamecoe/entity/camera.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <logcoe.hpp>
#include <glad/gl.h>

namespace gamecoe
{
    Camera::Camera(GameObject *owner, Window *window) : Component(owner), 
                                                        m_window(window), 
                                                        m_fov(45.0f),
                                                        m_nearPlane(0.1f),
                                                        m_farPlane(100.0f),
                                                        m_orthographicHeight(-1.0f),
                                                        m_perspective(true),
                                                        m_projectionCached(false)
    { 
        if (!window)
            detail::throwError("Window cannot be nullptr");
    }

    Camera::Camera(Camera &&other) noexcept : Component(other.m_owner), m_window(other.m_window),
                                              m_fov(other.m_fov), m_nearPlane(other.m_nearPlane),
                                              m_farPlane(other.m_farPlane), 
                                              m_orthographicHeight(other.m_orthographicHeight),
                                              m_perspective(other.m_perspective), 
                                              m_projectionCached(other.m_projectionCached),
                                              m_projectionMatrix(other.m_projectionMatrix)
    {
        m_active = other.m_active;

        other.m_owner = nullptr;
        other.m_window = nullptr;
        other.m_active = false;
    }

    Camera &Camera::operator=(Camera &&other) noexcept
    {
        if (this == &other) return *this;

        m_owner = other.m_owner;
        m_window = other.m_window;
        m_active = other.m_active;
        m_fov = other.m_fov;
        m_nearPlane = other.m_nearPlane;
        m_farPlane = other.m_farPlane;
        m_orthographicHeight = other.m_orthographicHeight;
        m_perspective = other.m_perspective;
        m_projectionCached = other.m_projectionCached;
        m_projectionMatrix = other.m_projectionMatrix;

        other.m_owner = nullptr;
        other.m_window = nullptr;
        other.m_active = false;

        return *this;
    }

    void Camera::activate() 
    { 
        m_active = true;
    }

    void Camera::deactivate() 
    { 
        m_active = false;
    }

    void Camera::setFov(float fov) 
    {
        if (fov <= 0.0f || 180.0f <= fov)
            detail::throwError("FOV must be between 0 and 180 degrees");

        m_fov = fov;
        m_projectionCached = false;
    }

    void Camera::setNearPlane(float nearPlane) 
    {
        if (nearPlane <= 0.0f)
            detail::throwError("Near plane must be positive");
        if (nearPlane >= m_farPlane)
            detail::throwError("Far plane must be greater than the near plane");

        m_nearPlane = nearPlane; 
        m_projectionCached = false;
    }

    void Camera::setFarPlane(float farPlane) 
    {
        if (farPlane <= 0.0f)
            detail::throwError("Far plane must be positive");
        if (farPlane <= m_nearPlane)
            detail::throwError("Far plane must be greater than the near plane");

        m_farPlane = farPlane;
        m_projectionCached = false;
    }

    void Camera::setPlanes(float nearPlane, float farPlane)
    {
        if (nearPlane <= 0.0f || farPlane <= 0.0f)
            detail::throwError("Both planes must be positive");
        if (nearPlane >= farPlane)
            detail::throwError("Far plane must be greater than the near plane");

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
            detail::throwError("Orthographic size must be positive");

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
        const auto &transform = m_owner->transform();
        const auto &position = transform.position();

        return glm::lookAt(position, position + transform.forward(), transform.up());
    }

    const glm::mat4 &Camera::projectionMatrix() const 
    { 
        if (!m_projectionCached)
        {
            if (m_perspective)
                m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_window->aspectRatio(), m_nearPlane, m_farPlane);
            else
            {
                float halfHeight = m_orthographicHeight / 2.0f;
                float halfWidth = halfHeight * m_window->aspectRatio();
                m_projectionMatrix = glm::ortho(-halfWidth, halfWidth, 
                                                -halfHeight, halfHeight,
                                                m_nearPlane, m_farPlane);
            }
            m_projectionCached = true;
        }

        return m_projectionMatrix;
    }
} // namespace gamecoe
