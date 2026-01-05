#pragma once

#include <string>
#include <stdexcept>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

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
