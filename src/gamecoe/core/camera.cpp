#include <gamecoe/core/camera.hpp>
#include <glad/gl.h>

namespace gamecoe
{
    void Camera::mouseCallback([[maybe_unused]] void *window, double x, double y)
    {
    }

    void Camera::scrollCallback([[maybe_unused]] void *window, [[maybe_unused]] double x, double y)
    {
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 forward;
        forward.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        forward.y = glm::sin(glm::radians(m_pitch));
        forward.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));

        m_forward = forward;
        m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_forward));
    }

    void Camera::processKeyboard(const CameraMovement &direction, float deltaTime)
    {
        float velocity = m_movementSpeed * deltaTime;
        glm::vec3 multiplier = (direction == CameraMovement::Forward || 
                                direction == CameraMovement::Backward) ? m_forward : m_right;
        if (direction == CameraMovement::Backward || direction == CameraMovement::Left)
            multiplier *= -1.0f;

        m_position += multiplier * velocity;
    }

    void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
    {
        xOffset *= m_mouseSensitivity;
        yOffset *= m_mouseSensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        if(constrainPitch)
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

        updateCameraVectors();
    }

    void Camera::processMouseScroll(float yOffset)
    {
        m_zoom -= yOffset;
        m_zoom = glm::clamp(m_zoom, 1.0f, 45.0f);
    }

    Camera::Camera()
    {
    }

    Camera::Camera(void *window, const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    {
    }

    Camera::Camera(const Camera &other)
    {
    }

    Camera &Camera::operator=(const Camera &other)
    {
    }

    Camera::Camera(Camera &&other) noexcept
    {
    }

    Camera &Camera::operator=(Camera &&other) noexcept
    {
    }

    Camera::~Camera()
    {
    }

    void Camera::update()
    {
    }

    glm::mat4 Camera::getViewMatrix() const
    {
    }

    void Camera::enableMouseMovement(bool inverseY, bool inverseX)
    {
    }

    void Camera::disableMouseMovement()
    {
    }

    void Camera::enableScrollZoom()
    {
    }

    void Camera::disableScrollZoom()
    {
    }

    void Camera::move(const glm::vec3 &movement)
    {
    }

    void Camera::rotate(const glm::vec3 &rotation)
    {
    }

    void Camera::inverse()
    {
    }
} // namespace gamecoe