#include <gamecoe/core/window.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <inputcoe.hpp>
#include <utility>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>
#include <gamecoe/utils/consts.hpp>

namespace gamecoe
{
    void window::framebuffer_size_callback([[maybe_unused]] GLFWwindow *glfwWindow, int width, int height)
    {
        m_width = width;
        m_height = height;

#if GAMECOE_USE_OPENGL
        glViewport(0, 0, width, height);
#endif
    }

    window::window(GLFWwindow *glfwWindow, const std::string &title, std::uint32_t width, std::uint32_t height)
        : m_window(glfwWindow), m_title(title), m_width(width), m_height(height), m_first_frame(true)
    {
        glfwSetWindowUserPointer(m_window, this);

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *glfwWindow, int width, int height) {
            window *windowPtr = static_cast<window*>(glfwGetWindowUserPointer(glfwWindow));
            GAMECOE_ASSERT_LOG(windowPtr, "window::framebuffer_size_callback(): user pointer is null");
            windowPtr->framebuffer_size_callback(glfwWindow, width, height);
        });
        glfwSetCursorPosCallback(m_window, inputcoe::detail::mousePositionCallback);
        glfwSetKeyCallback(m_window, inputcoe::detail::keyCallback);
        glfwSetMouseButtonCallback(m_window, inputcoe::detail::mouseButtonCallback);
    }

    window::window(window &&other) noexcept
        : m_window(other.m_window), m_title(std::move(other.m_title)), m_width(other.m_width),
          m_height(other.m_height), m_first_frame(other.m_first_frame)
    {
        other.m_window = nullptr;

        // resize callback dereferences the stored user pointer as window*, must point at the surviving object
        if(m_window)
            glfwSetWindowUserPointer(m_window, this);
    }

    window& window::operator=(window &&other) noexcept
    {
        if(this == &other)
            return *this;

        std::swap(m_window, other.m_window);
        std::swap(m_title, other.m_title);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_first_frame, other.m_first_frame);

        // resize callback dereferences the stored user pointer as window*, must point at the surviving object
        // (other now owns this's old GLFWwindow* and destroys it normally when it goes out of scope)
        if(m_window)
            glfwSetWindowUserPointer(m_window, this);
        if(other.m_window)
            glfwSetWindowUserPointer(other.m_window, &other);

        return *this;
    }

    window::~window()
    {
        if(!m_window) return;

        glfwSetWindowUserPointer(m_window, nullptr);
        glfwDestroyWindow(m_window);
    }

    std::expected<window, error> window::create(const std::string &title, std::uint32_t width, std::uint32_t height)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GAMECOE_GRAPHICS_VERSION_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GAMECOE_GRAPHICS_VERSION_MINOR);

#if GAMECOE_OPENGL_VERSION_AT_LEAST(3, 2)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GAMECOE_GRAPHICS_PROFILE);
#if defined(__APPLE__) && GAMECOE_OPENGL_VERSION_AT_LEAST(3, 0)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#endif
#if GAMECOE_USE_OPENGL
        glfwWindowHint(GLFW_SAMPLES, 4);
#endif

        GLFWwindow *current = glfwGetCurrentContext();
        GLFWwindow *glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, current);
        if(!glfwWindow)
            return std::unexpected(
                        detail::make_error(
                            error_code::window_creation_failed,
                            "window::create(): Failed to create glfw window"));

        if(!current) // multi-window support
        {
            glfwMakeContextCurrent(glfwWindow);
            glfwSwapInterval(0); // TODO: maybe in the future allow the users to limit their FPS
        }

        logcoe::info("window::create(): created window \"" + title + 
                        "\" (" + std::to_string(width) + "x" + std::to_string(height) + ")");

        return window{glfwWindow, title, width, height};
    }

    bool window::active()
    {
        GAMECOE_ASSERT_LOG(m_window, "window::active(): window is null");

        if(m_first_frame)
            m_first_frame = false;
        else
        {
#if GAMECOE_USE_OPENGL
            glfwSwapBuffers(m_window);
#endif
        }

        return !glfwWindowShouldClose(m_window);
    }

    float window::aspect_ratio() const { return (float)m_width / (float)m_height; }

} // namespace gamecoe
