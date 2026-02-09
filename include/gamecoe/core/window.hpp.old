#pragma once

#include <string>
#include <cstdint>

struct GLFWwindow;

namespace gamecoe
{
    class Window
    {
        GLFWwindow *m_window;
        
        std::string m_title;
        std::uint32_t m_width;
        std::uint32_t m_height;
        bool m_firstFrame;

        void framebufferSizeCallback(GLFWwindow *window, int width, int height);

    public:
        Window();
        Window(const std::string &title, std::uint32_t width, std::uint32_t height);
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = default;
        Window& operator=(Window&&) = default;
        ~Window();

        bool active();

        float aspectRatio() const;
    };
} // namespace gamecoe