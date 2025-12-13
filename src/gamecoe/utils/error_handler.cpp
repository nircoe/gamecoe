#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.h>

#if GAMECOE_USE_OPENGL
#include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace detail
    {
        void checkAndThrowError(const std::string &method)
        {
#if GAMECOE_USE_OPENGL
            GLenum error = glGetError();
            if(error != GL_NO_ERROR) 
                throwError(method + " OpenGL error: " + std::to_string(error));
#endif
        }

        void clearError()
        {
#if GAMECOE_USE_OPENGL
            glGetError();
#endif
        }
    } // namespace detail
} // namespace gamecoe