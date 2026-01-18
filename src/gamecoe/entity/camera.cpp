#include <gamecoe/entity/camera.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/core/window.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/consts.hpp>
#include <cmath>

namespace gamecoe
{
    void Camera::calculateFrustum() const
    {
        const auto &transform = m_owner.transform();

        const auto position = transform.worldPosition();
        const auto forward = transform.worldForward();
        const auto right = transform.worldRight();
        const auto up = transform.worldUp();

        const float aspectRatio = m_owner.game().mainWindow().aspectRatio();
        const float halfVSide = m_farPlane * std::tanf(glm::radians(m_fov) * 0.5f);
        const float halfHSide = halfVSide * aspectRatio;
        const auto forwardFar = m_farPlane * forward;

        m_frustum.m_topFace.m_normal = m_perspective ? glm::normalize(glm::cross(right, forwardFar - (halfVSide * up))) : -up;
        m_frustum.m_topFace.m_point = m_perspective ? position : position + (halfVSide * up);
        m_frustum.m_bottomFace.m_normal = m_perspective ? glm::normalize(glm::cross(forwardFar + (halfVSide * up), right)) : up;
        m_frustum.m_bottomFace.m_point = m_perspective ? position : position - (halfVSide * up);

        m_frustum.m_rightFace.m_normal = m_perspective ? glm::normalize(glm::cross(forwardFar - (halfHSide * right), up)) : -right;
        m_frustum.m_rightFace.m_point = m_perspective ? position : position + (halfHSide * right);
        m_frustum.m_leftFace.m_normal = m_perspective ? glm::normalize(glm::cross(up, forwardFar + (halfHSide * right))) : right;
        m_frustum.m_leftFace.m_point = m_perspective ? position : position - (halfHSide * right);

        m_frustum.m_nearFace.m_normal = forward;
        m_frustum.m_nearFace.m_point = position + (m_nearPlane * forward);
        m_frustum.m_farFace.m_normal = -forward;
        m_frustum.m_farFace.m_point = position + forwardFar;
    }

    Camera::Camera(GameObject &owner) : Component(owner), 
                                        m_fov(45.0f),
                                        m_nearPlane(0.1f),
                                        m_farPlane(100.0f),
                                        m_orthographicHeight(-1.0f),
                                        m_perspective(true),
                                        m_projectionCached(false),
                                        m_projectionMatrix(),
                                        m_frustumCached(false),
                                        m_frustum()
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
            glm::mat4 m_projection;
            glm::mat4 m_view;
            glm::vec3 m_cameraPosition;
        } data;
        data.m_projection = projectionMatrix();
        data.m_view = viewMatrix();
        data.m_cameraPosition = m_owner.transform().worldPosition();

        m_uniformBuffer->uploadData(&data, sizeof(data));
#endif
    }

    void Camera::setFov(float fov) 
    {
        if (fov <= 0.0f || 180.0f <= fov)
            detail::invalidArgument("Camera::setFov(): FOV must be between 0 and 180 degrees");

        m_fov = fov;
        m_projectionCached = m_frustumCached = false;
    }

    void Camera::setNearPlane(float nearPlane) 
    {
        if (nearPlane <= 0.0f)
            detail::invalidArgument("Camera::setNearPlane(): Near plane must be positive");
        if (nearPlane >= m_farPlane)
            detail::throwError("Camera::setNearPlane(): Far plane (" + std::to_string(m_farPlane) + 
                               ") must be greater than the near plane (" + std::to_string(nearPlane) + ")");

        m_nearPlane = nearPlane; 
        m_projectionCached = m_frustumCached = false;
    }

    void Camera::setFarPlane(float farPlane) 
    {
        if (farPlane <= 0.0f)
            detail::invalidArgument("Camera::setFarPlane(): Far plane must be positive");
        if (farPlane <= m_nearPlane)
            detail::throwError("Camera::setFarPlane(): Far plane (" + std::to_string(farPlane) + 
                               ") must be greater than the near plane (" + std::to_string(m_nearPlane) + ")");

        m_farPlane = farPlane;
        m_projectionCached = m_frustumCached = false;
    }

    void Camera::setPlanes(float nearPlane, float farPlane)
    {
        if (nearPlane <= 0.0f || farPlane <= 0.0f)
            detail::invalidArgument("Camera::setPlanes(): Both planes must be positive");
        if (nearPlane >= farPlane)
            detail::throwError("Camera::setPlanes(): Far plane must be greater than the near plane");

        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        m_projectionCached = m_frustumCached = false;
    }

    void Camera::setPerspectiveMode() 
    { 
        if (m_perspective) return;

        m_orthographicHeight = -1.0f;
        m_projectionCached = m_frustumCached = false;
        m_perspective = true;
    }

    void Camera::setOrthographicMode(float orthographicSize) 
    { 
        if (!m_perspective) return;

        if (orthographicSize <= 0.0f) 
            detail::invalidArgument("Camera::setOrthographicMode(): Orthographic size must be positive");

        m_orthographicHeight = orthographicSize;
        m_projectionCached = m_frustumCached = false;
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

    const Camera::Frustum &Camera::frustum() const
    {
        if (!m_frustumCached || m_owner.transform().modelChanged())
        {
            calculateFrustum();
            m_frustumCached = true;
        }

        return m_frustum;
    }
} // namespace gamecoe
