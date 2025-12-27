#include <timecoe.hpp>
#include <GLFW/glfw3.h>

namespace
{
    static float s_lastFrameTime = -1.0f;
    static float s_deltaTime = 0.0f;
}

namespace timecoe
{
    namespace detail
    {
        void update()
        {
            float time = glfwGetTime();

            if (s_lastFrameTime < 0.0f) s_lastFrameTime = time;

            s_deltaTime = time - s_lastFrameTime;
            s_lastFrameTime = time;
        }
    } // namespace detail

    float deltaTime() noexcept
    {
        return s_deltaTime;
    }
} // namespace timecoe
