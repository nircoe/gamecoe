#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gamecoe
{
    enum class camera_movement
    {
        Forward,
        Backward,
        Left,
        Right
    };

    class camera
    {
        void *m_window; // TODO: introduce gamecoe::window class

        glm::vec3 m_position;
        glm::vec3 m_forward;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;

        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_zoom;

        bool m_mouseMovementEnabled;
        bool m_scrollZoomEnabled;

        void mouse_callback([[maybe_unused]] void *window, double x, double y);
        void scroll_callback([[maybe_unused]] void *window, [[maybe_unused]] double x, double y);

        void update_camera_vectors();
        void process_keyboard(const camera_movement &direction, float deltaTime);
        void process_mouse_movement(float xOffset, float yOffset, bool constrainPitch = true);
        void process_mouse_scroll(float yOffset);

    public:
        camera();
        camera(void *window, const glm::vec3 &position, const glm::vec3 &target = glm::vec3(0.0f), const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));
        camera(const camera &other);
        camera &operator=(const camera &other);
        camera(camera &&other) noexcept;
        camera &operator=(camera &&other) noexcept;

        ~camera();

        void update();

        glm::mat4 get_view_matrix() const;

        void enable_mouse_movement(bool inverseY = false, bool inverseX = false);
        void disable_mouse_movement();

        void enable_scroll_zoom();
        void disable_scrool_zoom();

        void move(const glm::vec3 &movement);
        void rotate(const glm::vec3 &rotation);
        void inverse();
    };
} // namespace gamecoe