#pragma once

#include <string>
#include <cstdint>
#include <expected>
#include <gamecoe/utils/error.hpp>

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

        window(GLFWwindow *glfwWindow, const std::string &title, std::uint32_t width, std::uint32_t height);
        void framebuffer_size_callback(int width, int height);

    public:
        window(const window&) = delete;
        window& operator=(const window&) = delete;
        window(window &&other) noexcept;
        window& operator=(window &&other) noexcept;
        ~window();

        // Requires glfwInit() to have already succeeded (window does not call glfwInit()/glfwTerminate() itself - that's game's responsibility).
        [[nodiscard]] static std::expected<window, error> create(
            const std::string &title = "gamecoe", std::uint32_t width = 800, std::uint32_t height = 600);

        bool active();

        float aspect_ratio() const;
    };
} // namespace gamecoe
