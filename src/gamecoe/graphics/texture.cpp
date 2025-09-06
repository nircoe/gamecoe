#include <gamecoe/graphics/texture.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.h>
#include <stb/stb_image.h>
#include <logcoe.hpp>
#include <cassert>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

using namespace gamecoe::detail;

namespace gamecoe
{
    Texture::Texture(const std::string &image, bool flipVertically, bool generateMipmap)
    {
        struct GarbageCollector
        {
            unsigned char *m_data = nullptr;
            ~GarbageCollector()
            {
                if(m_data)
                    stbi_image_free(m_data);
            }
        } gb;

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(flipVertically);
        unsigned char *data = stbi_load(image.c_str(), &width, &height, &nrChannels, 0);
        if(!data)
            throwError("Texture::Texture(): Failed to load texture: " + image);
        gb.m_data = data;
        
#if GAMECOE_USE_OPENGL
        glGenTextures(1, &m_id);
        if(m_id == 0)
            throwError("Texture::Texture(): Failed to generate OpenGL texture");
        
        m_dimension = GL_TEXTURE_2D;
        glBindTexture(m_dimension, m_id);
    
        int format = (nrChannels == 1) ? GL_RED :
                     (nrChannels == 2) ? GL_RG :
                     (nrChannels == 3) ? GL_RGB :
                     (nrChannels == 4) ? GL_RGBA : 
                     0;
        if(format == 0)
            throwError("Texture::Texture(): Unsupported image channel count");

        glTexImage2D(m_dimension, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        checkAndThrowError();
        if(generateMipmap) 
        {
            glGenerateMipmap(m_dimension);
            checkAndThrowError();
        }

        glBindTexture(m_dimension, 0);
        clearError();
#endif
    }

    Texture::Texture(Texture &&other) noexcept : m_id(other.m_id), m_dimension(other.m_dimension)
    {
        other.m_id = 0;
    }

    Texture &Texture::operator=(Texture &&other) noexcept
    {
        if(this == &other) 
            return *this;

#if GAMECOE_USE_OPENGL
        if(m_id != 0)
            glDeleteTextures(1, &m_id);
#endif

        m_id = other.m_id;
        m_dimension = other.m_dimension;

        other.m_id = 0;

        return *this;
    }

    Texture::~Texture()
    {
#if GAMECOE_USE_OPENGL
        if(m_id != 0)
            glDeleteTextures(1, &m_id);
#endif
    }

    bool Texture::bind()
    {
#if GAMECOE_USE_OPENGL
        if(m_id == 0)
        {
            logcoe::warning("Texture::bind(): Cannot bind moved texture");
            return false;
        }

        glBindTexture(m_dimension, m_id);
        return true;
#else
        return false; // not supported yet
#endif
    }

    void Texture::unbind()
    {
#if GAMECOE_USE_OPENGL
        glBindTexture(m_dimension, 0);
#endif
    }

#if GAMECOE_USE_OPENGL
    static inline int parseTextureWrap(TextureWrap wrap) 
    { 
        return (wrap == TextureWrap::Repeat) ? GL_REPEAT :
               (wrap == TextureWrap::ClampToEdge) ? GL_CLAMP_TO_EDGE :
               (wrap == TextureWrap::ClampToBorder) ? GL_CLAMP_TO_BORDER : 
               GL_MIRRORED_REPEAT; 
    }

    static inline int parseTextureFilter(TextureFilter filter)
    {
        return (filter == TextureFilter::Nearest) ? GL_NEAREST :
               (filter == TextureFilter::Linear) ? GL_LINEAR :
               (filter == TextureFilter::NearestMipmapNearest) ? GL_NEAREST_MIPMAP_NEAREST :
               (filter == TextureFilter::NearestMipmapLinear) ? GL_NEAREST_MIPMAP_LINEAR :
               (filter == TextureFilter::LinearMipmapNearest) ? GL_LINEAR_MIPMAP_NEAREST :
               GL_LINEAR_MIPMAP_LINEAR;
    }

    static inline unsigned int getCurrentBoundTextureId(int dimension)
    {
        int textureBinding = (dimension == GL_TEXTURE_1D) ? GL_TEXTURE_BINDING_1D :
                             (dimension == GL_TEXTURE_2D) ? GL_TEXTURE_BINDING_2D :
                             GL_TEXTURE_BINDING_3D;
        GLint currentTexture;
        glGetIntegerv(textureBinding, &currentTexture);
        return static_cast<unsigned int>(currentTexture);
    }
#endif

    bool Texture::setParameters(TextureWrap wrapS, TextureWrap wrapT, TextureFilter minFilter, TextureFilter magFilter)
    {
#if GAMECOE_USE_OPENGL
        if(m_id == 0)
        {
            logcoe::warning("Texture::setParameters(): Cannot set parameters on moved texture");
            return false;
        }

        unsigned int currentTexture = getCurrentBoundTextureId(m_dimension);
        
        struct AutomaticRebinding
        {
            unsigned int m_texture;
            int m_dimension;
            bool m_shouldRebind = false;
            AutomaticRebinding(unsigned int tex, int dim) : m_texture(tex), m_dimension(dim) { }
            ~AutomaticRebinding()
            {
                if(m_shouldRebind)
                    glBindTexture(m_dimension, m_texture);
            }
        } rebind(currentTexture, m_dimension);

        if(currentTexture != m_id)
        {
            rebind.m_shouldRebind = true;
            bind();
        }

        int glWrapS = parseTextureWrap(wrapS);
        int glWrapT = parseTextureWrap(wrapT);
        int glMinFilter = parseTextureFilter(minFilter);
        int glMagFilter = parseTextureFilter(magFilter);

        glTexParameteri(m_dimension, GL_TEXTURE_WRAP_S, glWrapS);
        glTexParameteri(m_dimension, GL_TEXTURE_WRAP_T, glWrapT);
        glTexParameteri(m_dimension, GL_TEXTURE_MIN_FILTER, glMinFilter);
        glTexParameteri(m_dimension, GL_TEXTURE_MAG_FILTER, glMagFilter);
        try { checkAndThrowError(); }
        catch(...) { return false; }

        return true;
#else
        return false; // not supported yet
#endif
    }
} // namespace gamecoe