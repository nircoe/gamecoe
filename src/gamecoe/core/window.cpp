#include <gamecoe/core/window.hpp>
#include <gamecoe/core/time.hpp>
#include <logcoe.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <cassert>
#include <cstdlib>
#include <gamecoe_config.h>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

#include <GLFW/glfw3.h>
#include <gamecoe/utils/consts.hpp>

namespace gamecoe
{
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    Window::Window() : Window(800, 600, "gamecoe") { }

    Window::Window(size_t width, size_t height, const std::string &title) : m_window(nullptr), m_width(width), m_height(height), m_title(title), m_firstFrame(true), m_lastFrameTime(0.0f)
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
            detail::throwError("Failed to create glfw window");

        if(!current)
        {
            glfwMakeContextCurrent(window);

            #if GAMECOE_USE_OPENGL
                if(!gladLoadGL(glfwGetProcAddress))
                    detail::throwError("Failed to initialize glad");
            #endif
        }
        
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        m_window = window;
        gc.m_succeed = true;
    }

    Window::~Window()
    {
        if(!m_window) return;

        GLFWwindow *window = static_cast<GLFWwindow*>(m_window);
        glfwDestroyWindow(window);
    }

    bool Window::active()
    {
        assert(m_window);

        float time = glfwGetTime();
        GLFWwindow* window = static_cast<GLFWwindow*>(m_window);

        if(m_firstFrame)
        {
            m_lastFrameTime = time; // deltaTime == 0 in the first frame
            m_firstFrame = false;
        }
        else
        {
            #if GAMECOE_USE_OPENGL
                glfwSwapBuffers(window);
            #endif
            glfwPollEvents();
        }

        float deltaTime = time - m_lastFrameTime;
        detail::updateDeltaTime(deltaTime);
        assert(timecoe::deltaTime() == deltaTime);
        m_lastFrameTime = time;

        return !glfwWindowShouldClose(window);
    }

    float Window::aspectRatio() const { return (float)m_width / (float)m_height; }

} // namespace gamecoe