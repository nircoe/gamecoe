#include <timecoe.hpp>
#include <GLFW/glfw3.h>

namespace
{
    float g_last_frame_time = -1.0f;
    float g_delta_time = 0.0f;
}

namespace timecoe
{
    namespace detail
    {
        void update()
        {
            float time = static_cast<float>(glfwGetTime());

            if (g_last_frame_time < 0.0f) g_last_frame_time = time;

            g_delta_time = time - g_last_frame_time;
            g_last_frame_time = time;
        }
    } // namespace detail

    float delta_time() noexcept
    {
        return g_delta_time;
    }
} // namespace timecoe
