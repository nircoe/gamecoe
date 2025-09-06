#pragma once

#include <string>

namespace gamecoe
{
    enum class TextureWrap 
    { 
        Repeat, 
        ClampToEdge, 
        ClampToBorder, 
        MirroredRepeat 
    };

    enum class TextureFilter 
    { 
        Nearest, 
        Linear, 
        NearestMipmapNearest, 
        NearestMipmapLinear, 
        LinearMipmapNearest, 
        LinearMipmapLinear 
    };
    
    class Texture
    {
        unsigned int m_id;
        int m_dimension;

    public:
        Texture() = delete;
        // 2D Texture
        Texture(const std::string &image, bool flipVertically = true, bool generateMipmap = true);
        // TODO: Add designated constructor for 1D texture
        // TODO: Add designated constructor for 3D Texture
        Texture(const Texture&) = delete;
        Texture &operator=(const Texture&) = delete;
        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        ~Texture();

        bool bind();
        void unbind();

        bool setParameters(TextureWrap wrapS, TextureWrap wrapT, TextureFilter minFilter, TextureFilter magFilter);
    };
} // namespace gamecoe