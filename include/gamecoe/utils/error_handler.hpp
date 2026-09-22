#pragma once

#include <string>
#include <cassert>
#include <expected>
#include <gamecoe_config.hpp>
#include <gamecoe/utils/error.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

#define GAMECOE_ASSERT_LOG(condition, message) \
    do \
    { \
        const bool assert_cond = static_cast<bool>(condition); \
        if (!assert_cond) logcoe::error(message); \
        assert(assert_cond && message); \
    } while (0)

// Guarantees the Release build still returns a defined value instead of continuing past a violated precondition.
#define GAMECOE_ASSERT_GUARD(condition, message, ...) \
    do \
    { \
        const bool guard_cond = static_cast<bool>(condition); \
        if (!guard_cond) logcoe::error(message); \
        assert(guard_cond && message); \
        if (!guard_cond) return __VA_ARGS__; \
    } while (0)

namespace gamecoe
{
    namespace detail
    {
        // Centralizes the "always log on error construction" behavior.
        [[nodiscard]] error make_error(error_code code, const std::string &message);

        [[nodiscard]] inline error invalid_argument(const std::string &message)
        {
            return make_error(error_code::invalid_argument, message);
        }

        // Checks for API error (currently only OpenGL supported)
        [[nodiscard]] std::expected<void, error> check_error(const std::string &method);

        // Clears any existing API errors (currently only OpenGL supported)
        void clear_error();
    } // namespace detail
} // namespace gamecoe
