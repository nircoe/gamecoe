#pragma once

#include <string>
#include <cstdint>

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
        std::uint32_t m_id;
        std::int32_t m_dimension;

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

        void bind();
        void unbind();

        std::uint32_t id() const;
        std::int32_t dimension() const;

        bool setParameters(TextureWrap wrapS, TextureWrap wrapT, TextureFilter minFilter, TextureFilter magFilter);
    };
} // namespace gamecoe