#pragma once

#include <string>
#include <stdexcept>
#include <cassert>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

#define GAMECOE_ASSERT_LOG(condition, message) \
    do { if (!(condition)) logcoe::error(message); assert((condition) && message); } while (0)

namespace gamecoe
{
    namespace detail
    {
        [[noreturn]] static inline void throwError(const std::string &message)
        {
            logcoe::error(message);
            throw std::runtime_error(message);
        }

        [[noreturn]] static inline void invalidArgument(const std::string &message)
        {
            logcoe::error(message);
            throw std::invalid_argument(message);
        }

        // Checks for API error and throws if exists (currently only OpenGL supported)
        void checkAndThrowError(const std::string &method);

        // Clears any existing API errors (currently only OpenGL supported)
        void clearError();
    } // namespace detail
} // namespace gamecoe
