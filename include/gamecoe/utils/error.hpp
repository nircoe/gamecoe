#pragma once

#include <string>

namespace gamecoe
{
    enum class error_code
    {
        invalid_format,
        opengl_error,
        path_resolution_failed,
        unsupported_platform
    };

    struct error
    {
        error_code code;
        std::string message;
    };
} // namespace gamecoe
