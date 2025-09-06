#include <gamecoe/core/time.hpp>

namespace
{
    static float s_deltaTime = 0.0f;
}

namespace gamecoe
{
    namespace detail
    {
        void updateDeltaTime(float deltaTime)
        {
            s_deltaTime = deltaTime;
        }
    } // namespace detail
} // namespace gamecoe

namespace timecoe
{
    float deltaTime() noexcept
    {
        return s_deltaTime;
    }
} // namespace timecoe