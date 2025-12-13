#pragma once

namespace gamecoe
{
    namespace detail
    {
        void updateDeltaTime(float deltaTime);
    } // namespace detail
} // namespace gamecoe

namespace timecoe
{
    float deltaTime() noexcept;
} // namespace timecoe
