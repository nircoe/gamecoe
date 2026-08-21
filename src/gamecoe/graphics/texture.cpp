#include <gamecoe/graphics/texture.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <stb/stb_image.h>
#include <cmath>
#include <algorithm>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace graphics
    {
        namespace
        {
#if GAMECOE_USE_OPENGL
            int parse_texture_wrap(texture_wrap wrap)
            {
                return (wrap == texture_wrap::repeat) ? GL_REPEAT :
                       (wrap == texture_wrap::clamp_to_edge) ? GL_CLAMP_TO_EDGE :
                       (wrap == texture_wrap::clamp_to_border) ? GL_CLAMP_TO_BORDER :
                       GL_MIRRORED_REPEAT;
            }

            int parse_texture_filter(texture_filter filter)
            {
                return (filter == texture_filter::nearest) ? GL_NEAREST :
                       (filter == texture_filter::linear) ? GL_LINEAR :
                       (filter == texture_filter::nearest_mipmap_nearest) ? GL_NEAREST_MIPMAP_NEAREST :
                       (filter == texture_filter::nearest_mipmap_linear) ? GL_NEAREST_MIPMAP_LINEAR :
                       (filter == texture_filter::linear_mipmap_nearest) ? GL_LINEAR_MIPMAP_NEAREST :
                       GL_LINEAR_MIPMAP_LINEAR;
            }

#if !GAMECOE_HAS_DSA
            std::uint32_t get_current_bound_texture_id(int dimension)
            {
                int texture_binding = (dimension == GL_TEXTURE_1D) ? GL_TEXTURE_BINDING_1D :
                                      (dimension == GL_TEXTURE_2D) ? GL_TEXTURE_BINDING_2D :
                                      GL_TEXTURE_BINDING_3D;
                GLint current_texture;
                glGetIntegerv(texture_binding, &current_texture);
                return static_cast<std::uint32_t>(current_texture);
            }
#endif
#endif
        } // namespace

        texture::texture(std::uint32_t id, std::int32_t dimension) : m_id(id), m_dimension(dimension) { }

        texture::texture(texture &&other) noexcept : m_id(other.m_id), m_dimension(other.m_dimension)
        {
            other.reset();
        }

        texture &texture::operator=(texture &&other) noexcept
        {
            if (this == &other)
                return *this;

            destroy();

            m_id = other.m_id;
            m_dimension = other.m_dimension;

            other.reset();

            return *this;
        }

        void texture::destroy()
        {
#if GAMECOE_USE_OPENGL
            if (m_id != 0)
                glDeleteTextures(1, &m_id);
#endif
        }

        void texture::reset() noexcept
        {
            m_id = 0;
            m_dimension = 0;
        }

        texture::~texture()
        {
            destroy();
        }

        std::expected<texture, error> texture::create_2d([[maybe_unused]] const std::string &image,
                                                        [[maybe_unused]] bool flip_vertically,
                                                        [[maybe_unused]] bool generate_mipmap)
        {
#if GAMECOE_USE_OPENGL
            struct garbage_collector
            {
                unsigned char *m_data = nullptr;
                std::uint32_t m_id = 0;
                ~garbage_collector()
                {
                    if (m_data)
                        stbi_image_free(m_data);
                    if (m_id != 0)
                        glDeleteTextures(1, &m_id);
                }
            } gc;

            int width, height, nr_channels;
            stbi_set_flip_vertically_on_load(flip_vertically);
            unsigned char *data = stbi_load(image.c_str(), &width, &height, &nr_channels, 0);
            if (!data)
                return std::unexpected(
                        detail::make_error(
                            error_code::image_load_failure,
                            "texture::create_2d(): Failed to load texture: " + image));
            gc.m_data = data;

            std::uint32_t id = 0;
#if GAMECOE_HAS_DSA
            glCreateTextures(GL_TEXTURE_2D, 1, &id);
#else
            glGenTextures(1, &id);
#endif
            if (id == 0)
                return std::unexpected(
                        detail::make_error(
                            error_code::resource_creation_failure,
                            "texture::create_2d(): Failed to generate OpenGL texture"));
            gc.m_id = id;

            std::int32_t dimension = GL_TEXTURE_2D;
#if !GAMECOE_HAS_DSA
            glBindTexture(dimension, id);
#endif

            int format = (nr_channels == 1) ? GL_RED :
                         (nr_channels == 2) ? GL_RG :
                         (nr_channels == 3) ? GL_RGB :
                         (nr_channels == 4) ? GL_RGBA : 0;
            int internal_format = (nr_channels == 1) ? GL_R8 :
                                  (nr_channels == 2) ? GL_RG8 :
                                  (nr_channels == 3) ? GL_RGB8 :
                                  (nr_channels == 4) ? GL_RGBA8 : 0;
            if (format == 0 || internal_format == 0)
                return std::unexpected(
                        detail::make_error(
                            error_code::invalid_argument,
                            "texture::create_2d(): Unsupported image channel count"));

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#if GAMECOE_HAS_DSA
            int levels = generate_mipmap ? static_cast<int>(1 + std::floor(std::log2(std::max(width, height)))) : 1;
            glTextureStorage2D(id, levels, internal_format, width, height);
            glTextureSubImage2D(id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
#else
            glTexImage2D(dimension, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
#endif
            auto result = detail::check_error("texture::create_2d():");
            if (!result)
                return std::unexpected(result.error());

            if (generate_mipmap)
            {
#if GAMECOE_HAS_DSA
                glGenerateTextureMipmap(id);
#else
                glGenerateMipmap(dimension);
#endif
                result = detail::check_error("texture::create_2d():");
                if (!result)
                    return std::unexpected(result.error());
            }

#if !GAMECOE_HAS_DSA
            glBindTexture(dimension, 0);
#endif
            detail::clear_error();

            gc.m_id = 0;
            return texture{id, dimension};
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "texture::create_2d(): Only OpenGL supported at the moment"));
#endif
        }

        void texture::bind() const
        {
#if GAMECOE_USE_OPENGL && !GAMECOE_HAS_DSA
            GAMECOE_ASSERT_LOG(m_id != 0, "texture::bind(): cannot bind a moved-from texture");

            glBindTexture(m_dimension, m_id);
#endif
        }

        void texture::unbind() const
        {
#if GAMECOE_USE_OPENGL && !GAMECOE_HAS_DSA
            GAMECOE_ASSERT_LOG(m_dimension != 0, "texture::unbind(): cannot unbind a moved-from texture");

            glBindTexture(m_dimension, 0);
#endif
        }

        std::uint32_t texture::id() const
        {
            return m_id;
        }

        std::int32_t texture::dimension() const
        {
            return m_dimension;
        }

        std::expected<void, error> texture::set_parameters([[maybe_unused]] texture_wrap wrap_s,
                                                             [[maybe_unused]] texture_wrap wrap_t,
                                                             [[maybe_unused]] texture_filter min_filter,
                                                             [[maybe_unused]] texture_filter mag_filter)
        {
#if GAMECOE_USE_OPENGL
            GAMECOE_ASSERT_LOG(m_id != 0, "texture::set_parameters(): cannot set parameters on a moved-from texture");

#if !GAMECOE_HAS_DSA
            std::uint32_t current_texture = get_current_bound_texture_id(m_dimension);

            struct automatic_rebinding
            {
                std::uint32_t m_texture;
                int m_dimension;
                bool m_should_rebind = false;
                automatic_rebinding(std::uint32_t tex, int dim) : m_texture(tex), m_dimension(dim) { }
                ~automatic_rebinding()
                {
                    if (m_should_rebind)
                        glBindTexture(m_dimension, m_texture);
                }
            } rebind(current_texture, m_dimension);

            if (current_texture != m_id)
            {
                rebind.m_should_rebind = true;
                bind();
            }
#endif

            int gl_wrap_s = parse_texture_wrap(wrap_s);
            int gl_wrap_t = parse_texture_wrap(wrap_t);
            int gl_min_filter = parse_texture_filter(min_filter);
            int gl_mag_filter = parse_texture_filter(mag_filter);

#if GAMECOE_HAS_DSA
            glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, gl_wrap_s);
            glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, gl_wrap_t);
            glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, gl_min_filter);
            glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
#else
            glTexParameteri(m_dimension, GL_TEXTURE_WRAP_S, gl_wrap_s);
            glTexParameteri(m_dimension, GL_TEXTURE_WRAP_T, gl_wrap_t);
            glTexParameteri(m_dimension, GL_TEXTURE_MIN_FILTER, gl_min_filter);
            glTexParameteri(m_dimension, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
#endif
            return detail::check_error("texture::set_parameters():");
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "texture::set_parameters(): Only OpenGL supported at the moment"));
#endif
        }
    } // namespace graphics
} // namespace gamecoe
