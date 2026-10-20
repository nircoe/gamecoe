#pragma once

namespace timecoe
{
    namespace detail
    {
        void update();
    } // namespace detail

    [[nodiscard]] float delta_time() noexcept;
} // namespace timecoe
