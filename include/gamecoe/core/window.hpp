#pragma once

#include <string>

namespace gamecoe
{
    class Window
    {
        void *m_window;
        size_t m_width;
        size_t m_height;
        std::string m_title;
        bool m_firstFrame;
        float m_lastFrameTime;

    public:
        Window();
        Window(size_t width, size_t height, const std::string &title);
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = default;
        Window& operator=(Window&&) = default;
        ~Window();

        bool active();

        float aspectRatio() const;
    };
} // namespace gamecoe