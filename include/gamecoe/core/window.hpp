#pragma once

#include <string>
#include <cstdint>

namespace gamecoe
{
    class Window
    {
        void *m_window;
        
        std::string m_title;
        std::uint32_t m_width;
        std::uint32_t m_height;
        bool m_firstFrame;

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