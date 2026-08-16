#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace detail
    {
        error make_error(error_code code, const std::string &message)
        {
            logcoe::error(message);
            return error{code, message};
        }

        std::expected<void, error> check_error(const std::string &method)
        {
#if GAMECOE_USE_OPENGL
            GLenum err = glGetError();
            if(err != GL_NO_ERROR)
                return std::unexpected(make_error(error_code::opengl_error, method + " OpenGL error: " + std::to_string(err)));
#endif
            return {};
        }

        void clear_error()
        {
#if GAMECOE_USE_OPENGL
            glGetError();
#endif
        }
    } // namespace detail
} // namespace gamecoe
