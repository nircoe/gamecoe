#include <gamecoe/graphics/texture.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <stb/stb_image.h>
#include <cassert>
#include <cmath>
#include <algorithm>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

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
            detail::throwError("Texture::Texture(): Failed to load texture: " + image);
        gb.m_data = data;
        
#if GAMECOE_USE_OPENGL
#if GAMECOE_HAS_DSA
        glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
#else
        glGenTextures(1, &m_id);
#endif
        if(m_id == 0)
            detail::throwError("Texture::Texture(): Failed to generate OpenGL texture");
        
        m_dimension = GL_TEXTURE_2D;
#if !GAMECOE_HAS_DSA
        glBindTexture(m_dimension, m_id);
#endif
    
        int format = (nrChannels == 1) ? GL_RED :
                     (nrChannels == 2) ? GL_RG :
                     (nrChannels == 3) ? GL_RGB :
                     (nrChannels == 4) ? GL_RGBA : 0;
        int internalFormat = (nrChannels == 1) ? GL_R8 :
                             (nrChannels == 2) ? GL_RG8 :
                             (nrChannels == 3) ? GL_RGB8 :
                             (nrChannels == 4) ? GL_RGBA8 : 0;
        if(format == 0 || internalFormat == 0) // checking both just in case
            detail::throwError("Texture::Texture(): Unsupported image channel count");

#if GAMECOE_HAS_DSA
        int levels = generateMipmap ? (1 + std::floor(std::log2(std::max(width, height)))) : 1;
        glTextureStorage2D(m_id, levels, internalFormat, width, height);
        glTextureSubImage2D(m_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
#else
        glTexImage2D(m_dimension, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
#endif
        detail::checkAndThrowError("Texture::Texture():");
        if(generateMipmap) 
        {
#if GAMECOE_HAS_DSA
            glGenerateTextureMipmap(m_id);
#else
            glGenerateMipmap(m_dimension);
#endif
            detail::checkAndThrowError("Texture::Texture():");
        }

#if !GAMECOE_HAS_DSA
        glBindTexture(m_dimension, 0);
#endif
        detail::clearError();
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

    void Texture::bind()
    {
#if !GAMECOE_HAS_DSA
        if(m_id == 0)
            assert(false && "Texture::bind(): Cannot bind moved texture");

        glBindTexture(m_dimension, m_id);
#endif
    }

    void Texture::unbind()
    {
#if !GAMECOE_HAS_DSA
        glBindTexture(m_dimension, 0);
#endif
    }

    std::uint32_t Texture::id() const
    {
        return m_id;
    }
    
    std::int32_t Texture::dimension() const
    {
        return m_dimension;
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
            assert(false && "Texture::setParameters(): Cannot set parameters on moved texture");
            return false;
        }

#if !GAMECOE_HAS_DSA
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
#endif

        int glWrapS = parseTextureWrap(wrapS);
        int glWrapT = parseTextureWrap(wrapT);
        int glMinFilter = parseTextureFilter(minFilter);
        int glMagFilter = parseTextureFilter(magFilter);

#if GAMECOE_HAS_DSA
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, glWrapS);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, glWrapT);
        glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, glMinFilter);
        glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, glMagFilter);
#else
        glTexParameteri(m_dimension, GL_TEXTURE_WRAP_S, glWrapS);
        glTexParameteri(m_dimension, GL_TEXTURE_WRAP_T, glWrapT);
        glTexParameteri(m_dimension, GL_TEXTURE_MIN_FILTER, glMinFilter);
        glTexParameteri(m_dimension, GL_TEXTURE_MAG_FILTER, glMagFilter);
#endif
        try { detail::checkAndThrowError("Texture::setParameters():"); }
        catch(...) { return false; }

        return true;
#endif
    }
} // namespace gamecoe