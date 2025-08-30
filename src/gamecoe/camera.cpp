#include <gamecoe/camera.hpp>
#include <glad/gl.h>

namespace gamecoe
{
    void camera::mouse_callback([[maybe_unused]] void *window, double x, double y)
    {
    }

    void camera::scroll_callback([[maybe_unused]] void *window, [[maybe_unused]] double x, double y)
    {
    }

    void camera::update_camera_vectors()
    {
        glm::vec3 forward;
        forward.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        forward.y = glm::sin(glm::radians(m_pitch));
        forward.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));

        m_forward = forward;
        m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_forward));
    }

    void camera::process_keyboard(const camera_movement &direction, float deltaTime)
    {
        float velocity = m_movementSpeed * deltaTime;
        glm::vec3 multiplier = (direction == camera_movement::Forward || 
                                direction == camera_movement::Backward) ? m_forward : m_right;
        if (direction == camera_movement::Backward || direction == camera_movement::Left)
            multiplier *= -1.0f;

        m_position += multiplier * velocity;
    }

    void camera::process_mouse_movement(float xOffset, float yOffset, bool constrainPitch)
    {
        xOffset *= m_mouseSensitivity;
        yOffset *= m_mouseSensitivity;

        m_yaw += xOffset;
        m_pitch += yOffset;

        if(constrainPitch)
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

        update_camera_vectors();
    }

    void camera::process_mouse_scroll(float yOffset)
    {
        m_zoom -= yOffset;
        m_zoom = glm::clamp(m_zoom, 1.0f, 45.0f);
    }

    camera::camera()
    {
    }

    camera::camera(void *window, const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    {
    }

    camera::camera(const camera &other)
    {
    }

    camera &camera::operator=(const camera &other)
    {
    }

    camera::camera(camera &&other) noexcept
    {
    }

    camera &camera::operator=(camera &&other) noexcept
    {
    }

    camera::~camera()
    {
    }

    void camera::update()
    {
    }

    glm::mat4 camera::get_view_matrix() const
    {
    }

    void camera::enable_mouse_movement(bool inverseY, bool inverseX)
    {
    }

    void camera::disable_mouse_movement()
    {
    }

    void camera::enable_scroll_zoom()
    {
    }

    void camera::disable_scrool_zoom()
    {
    }

    void camera::move(const glm::vec3 &movement)
    {
    }

    void camera::rotate(const glm::vec3 &rotation)
    {
    }

    void camera::inverse()
    {
    }
} // namespace gamecoe