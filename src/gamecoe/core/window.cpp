#include <gamecoe/core/window.hpp>
#include <gamecoe/core/time.hpp>
#include <logcoe.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <gamecoe_config.h>
#include <stdexcept>
#include <cassert>

namespace gamecoe
{

    static inline void throwError(const std::string &message)
    {
        logcoe::error(message);
        throw std::runtime_error(message);
    }

    Window::Window() : Window(800, 600, "gamecoe") { }

    Window::Window(size_t width, size_t height, const std::string &title) : m_window(nullptr), m_width(width), m_height(height), m_title(title), m_firstFrame(true), m_lastFrameTime(0.0f)
    {
        struct GarbageCollector
        {
            bool m_succeed = false;
            ~GarbageCollector()
            {
                if(m_succeed) return;
                
                // Window creation failed
                logcoe::shutdown();
                glfwTerminate();
            }
        } gc;

        logcoe::initialize(logcoe::LogLevel::INFO, "gamecoe"); // maybe move to "Game" class later?
        if(!glfwInit()) // maybe move to "Game" class later?
            throwError("Failed to initialize glfw");

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
            throwError("Failed to create glfw window");

        if(!current)
        {
            glfwMakeContextCurrent(window);

            #if GAMECOE_USE_OPENGL
                if(!gladLoadGL(glfwGetProcAddress)) // if glfwInit is moved to "Game" class, when do we will load glad? loading glad needs an active glfw window?
                    throwError("Failed to initialize glad");
            #endif
        }

        m_window = window;
        gc.m_succeed = true;
    }

    Window::~Window()
    {
        if(!m_window) return;

        GLFWwindow *window = static_cast<GLFWwindow*>(m_window);
        glfwDestroyWindow(window);
        logcoe::shutdown(); // move to "Game" class later?
        // glfwTerminate(); // when to do it? in a "Game" class (that will manage the game loop?)
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

} // namespace gamecoe