#include <gamecoe/graphics/buffer.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace graphics
    {
        buffer::buffer(std::uint32_t id, std::uint32_t target)
            : m_allocated_size(0), m_id(id), m_target(target) { }

        buffer::buffer(buffer &&other) noexcept
            : m_allocated_size(other.m_allocated_size), m_id(other.m_id), m_target(other.m_target)
        {
            other.reset();
        }

        buffer &buffer::operator=(buffer &&other) noexcept
        {
            if (this == &other)
                return *this;

            destroy();

            m_allocated_size = other.m_allocated_size;
            m_id = other.m_id;
            m_target = other.m_target;

            other.reset();

            return *this;
        }

        void buffer::destroy()
        {
#if GAMECOE_USE_OPENGL
            if (m_id != 0)
                glDeleteBuffers(1, &m_id);
#endif
        }

        void buffer::reset() noexcept
        {
            m_allocated_size = 0;
            m_id = 0;
            m_target = 0;
        }

        buffer::~buffer()
        {
            destroy();
        }

        std::expected<buffer, error> buffer::create([[maybe_unused]] std::uint32_t target,
                                                    [[maybe_unused]] std::uint32_t binding_point)
        {
#if GAMECOE_USE_OPENGL
#if !GAMECOE_HAS_UBO
            if (target == GL_UNIFORM_BUFFER)
                return std::unexpected(
                        detail::make_error(
                            error_code::unsupported_feature,
                            "buffer::create(): OpenGL version doesn't support Uniform Buffer"));
#endif
            std::uint32_t id = 0;
#if GAMECOE_HAS_DSA
            glCreateBuffers(1, &id);
#else
            glGenBuffers(1, &id);
#endif
            if (id == 0)
                return std::unexpected(
                        detail::make_error(
                            error_code::resource_creation_failure,
                            "buffer::create(): Could not generate buffer"));

            if (target != GL_UNIFORM_BUFFER)
                return buffer{id, target};

            glBindBufferBase(target, binding_point, id);
            auto result = detail::check_error("buffer::create(): Uniform Buffer:");
            if (!result)
            {
                glDeleteBuffers(1, &id);
                return std::unexpected(result.error());
            }

            return buffer{id, target};
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "buffer::create(): Only OpenGL supported at the moment"));
#endif
        }

        void buffer::bind() const
        {
#if GAMECOE_USE_OPENGL && !GAMECOE_HAS_DSA
            glBindBuffer(m_target, m_id);
#endif
        }

        void buffer::unbind() const
        {
#if GAMECOE_USE_OPENGL && !GAMECOE_HAS_DSA
            glBindBuffer(m_target, 0);
#endif
        }

        std::expected<void, error> buffer::upload_data([[maybe_unused]] const void *data, 
                                                        [[maybe_unused]] std::size_t size)
        {
#if GAMECOE_USE_OPENGL
            if (size > m_allocated_size)
            {
                std::uint32_t usage = (m_target == GL_UNIFORM_BUFFER) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
#if GAMECOE_HAS_DSA
                glNamedBufferData(m_id, size, nullptr, usage);
#else
                bind();
                glBufferData(m_target, size, nullptr, usage);
#endif
                auto result = detail::check_error("buffer::upload_data():");
                if (!result)
                    return result;
                m_allocated_size = size;
            }

#if GAMECOE_HAS_DSA
            glNamedBufferSubData(m_id, 0, size, data);
#else
            bind();
            glBufferSubData(m_target, 0, size, data);
#endif
            return detail::check_error("buffer::upload_data():");
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "buffer::upload_data(): Only OpenGL supported at the moment"));
#endif
        }

        std::uint32_t buffer::id() const
        {
            return m_id;
        }

        std::uint32_t buffer::target() const
        {
            return m_target;
        }

        std::expected<buffer, error> buffer::create_vertex_buffer()
        {
            return create(GL_ARRAY_BUFFER);
        }

        std::expected<buffer, error> buffer::create_index_buffer()
        {
            return create(GL_ELEMENT_ARRAY_BUFFER);
        }

        std::expected<buffer, error> buffer::create_uniform_buffer(std::uint32_t binding_point)
        {
            return create(GL_UNIFORM_BUFFER, binding_point);
        }
    } // namespace graphics
} // namespace gamecoe
