#pragma once

#include <string>
#include <stdexcept>
#include <logcoe.hpp>

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

        void checkAndThrowError(const std::string &method);

        void clearError();
    } // namespace detail
} // namespace gamecoe