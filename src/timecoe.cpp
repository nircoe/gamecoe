#include <timecoe.hpp>
#include <GLFW/glfw3.h>

namespace
{
    float g_lastFrameTime = -1.0f;
    float g_deltaTime = 0.0f;
}

namespace timecoe
{
    namespace detail
    {
        void update()
        {
            float time = static_cast<float>(glfwGetTime());

            if (g_lastFrameTime < 0.0f) g_lastFrameTime = time;

            g_deltaTime = time - g_lastFrameTime;
            g_lastFrameTime = time;
        }
    } // namespace detail

    float deltaTime() noexcept
    {
        return g_deltaTime;
    }
} // namespace timecoe
