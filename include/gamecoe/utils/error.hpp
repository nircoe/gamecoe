#pragma once

#include <string>

namespace gamecoe
{
    enum class error_code
    {
        file_read_failure,
        image_load_failure,
        invalid_argument,
        opengl_error,
        path_resolution_failure,
        resource_creation_failure,
        shader_compilation_failure,
        shader_link_failure,
        unsupported_feature,
        unsupported_platform,
        window_creation_failure
    };

    struct error
    {
        error_code code;
        std::string message;
    };
} // namespace gamecoe
