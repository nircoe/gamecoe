#pragma once

#include <string>

namespace gamecoe
{
    enum class error_code
    {
        invalid_argument,
        opengl_error,
        path_resolution_failed,
        resource_creation_failed,
        unsupported_feature,
        unsupported_platform,
        window_creation_failed
    };

    struct error
    {
        error_code code;
        std::string message;
    };
} // namespace gamecoe
