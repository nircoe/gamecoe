#pragma once

#include <gamecoe/utils/error.hpp>
#include <cstdint>
#include <expected>
#include <string>

struct GLFWwindow;

namespace gamecoe
{
    class window
    {
        GLFWwindow *m_window;

        std::string m_title;
        std::uint32_t m_width;
        std::uint32_t m_height;
        bool m_first_frame;

        window(GLFWwindow *glfw_window, const std::string &title, std::uint32_t width, std::uint32_t height);
        void framebuffer_size_callback(int width, int height);

    public:
        window(const window &) = delete;
        window &operator=(const window &) = delete;
        window(window &&other) noexcept;
        window &operator=(window &&other) noexcept;
        ~window();

        // Needs glfwInit() to have succeeded. game calls glfwInit()/glfwTerminate(), window doesn't.
        [[nodiscard]] static std::expected<window, error> create(const std::string &title = "gamecoe",
                                                                 std::uint32_t width = 800,
                                                                 std::uint32_t height = 600);

        bool active();

        [[nodiscard]] float aspect_ratio() const noexcept;
    };
} // namespace gamecoe
