#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gamecoe
{
    enum class CameraMovement
    {
        Forward,
        Backward,
        Left,
        Right
    };

    class Camera
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

        void mouseCallback([[maybe_unused]] void *window, double x, double y);
        void scrollCallback([[maybe_unused]] void *window, [[maybe_unused]] double x, double y);

        void updateCameraVectors();
        void processKeyboard(const CameraMovement &direction, float deltaTime);
        void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
        void processMouseScroll(float yOffset);

    public:
        Camera();
        Camera(void *window, const glm::vec3 &position, const glm::vec3 &target = glm::vec3(0.0f), const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));
        Camera(const Camera &other);
        Camera &operator=(const Camera &other);
        Camera(Camera &&other) noexcept;
        Camera &operator=(Camera &&other) noexcept;

        ~Camera();

        void update();

        glm::mat4 getViewMatrix() const;

        void enableMouseMovement(bool inverseY = false, bool inverseX = false);
        void disableMouseMovement();

        void enableScrollZoom();
        void disableScrollZoom();

        void move(const glm::vec3 &movement);
        void rotate(const glm::vec3 &rotation);
        void inverse();
    };
} // namespace gamecoe