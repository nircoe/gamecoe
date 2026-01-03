#include <gamecoe/core/window.hpp>
#include <timecoe.hpp>
#include <inputcoe.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <cassert>
#include <cstdlib>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

#include <GLFW/glfw3.h>
#include <gamecoe/utils/consts.hpp>

namespace gamecoe
{
    void Window::framebufferSizeCallback([[maybe_unused]] GLFWwindow *window, int width, int height)
    {
        m_width = width;
        m_height = height;

        #if GAMECOE_USE_OPENGL
            glViewport(0, 0, width, height);
        #endif
    }

    Window::Window() : Window("gamecoe", 800, 600) { }

    Window::Window(const std::string &title, std::uint32_t width, std::uint32_t height) : m_window(nullptr), m_title(title), m_width(width), m_height(height), m_firstFrame(true)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GAMECOE_GRAPHICS_VERSION_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GAMECOE_GRAPHICS_VERSION_MINOR);
        
        #if GAMECOE_USE_OPENGL
            glfwWindowHint(GLFW_OPENGL_PROFILE, GAMECOE_GRAPHICS_PROFILE);
            #ifdef __APPLE__
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            #endif
        #endif

        GLFWwindow *current = glfwGetCurrentContext();
        GLFWwindow *window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, current);
        if(!window)
            detail::throwError("Window::Window(): Failed to create glfw window");

        if(!current) // multi-window support
            glfwMakeContextCurrent(window);
        
        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            Window *windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));
            assert(windowPtr);
            windowPtr->framebufferSizeCallback(window, width, height);
        });
        glfwSetCursorPosCallback(window, inputcoe::detail::mousePositionCallback);
        glfwSetKeyCallback(window, inputcoe::detail::keyCallback);
        glfwSetMouseButtonCallback(window, inputcoe::detail::mouseButtonCallback);

        m_window = window;
    }

    Window::~Window()
    {
        if(!m_window) return;

        glfwSetWindowUserPointer(m_window, nullptr);
        glfwDestroyWindow(m_window);
    }

    bool Window::active()
    {
        assert(m_window);

        if(m_firstFrame)
            m_firstFrame = false;
        else
        {
            #if GAMECOE_USE_OPENGL
                glfwSwapBuffers(m_window);
            #endif
        }

        return !glfwWindowShouldClose(m_window);
    }

    float Window::aspectRatio() const { return (float)m_width / (float)m_height; }

} // namespace gamecoe