#pragma once

#include <string>
#include <stdexcept>
#include <logcoe.hpp>

namespace gamecoe
{
    namespace detail
    {
        static inline void throwError(const std::string &message)
        {
            logcoe::error(message);
            throw std::runtime_error(message);
        }

        void checkAndThrowError();

        void clearError();
    } // namespace detail
} // namespace gamecoe