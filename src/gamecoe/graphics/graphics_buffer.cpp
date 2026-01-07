#include <gamecoe/graphics/graphics_buffer.hpp>
#include <gamecoe_config.hpp>
#include <cassert>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

namespace gamecoe
{
    GraphicsBuffer::GraphicsBuffer(unsigned int target) : m_id(0), m_target(target)
    {
#if GAMECOE_HAS_DSA
        glCreateBuffers(1, &m_id);
#elif GAMECOE_USE_OPENGL
        glGenBuffers(1, &m_id);
#endif
    }

    GraphicsBuffer::~GraphicsBuffer()
    {
#if GAMECOE_USE_OPENGL
        glDeleteBuffers(1, &m_id);
#endif
        m_id = 0;
        m_target = 0;
    }

    GraphicsBuffer::GraphicsBuffer(GraphicsBuffer &&other) noexcept : m_id(other.m_id), m_target(other.m_target)
    {
        other.m_id = 0;
        other.m_target = 0;
    }

    GraphicsBuffer &GraphicsBuffer::operator=(GraphicsBuffer &&other) noexcept
    {
        if (this == &other) return *this;

#if GAMECOE_USE_OPENGL
        glDeleteBuffers(1, &m_id);
#endif
        m_id = other.m_id;
        m_target = other.m_target;
        other.m_id = 0;
        other.m_target = 0;

        return *this;
    }

    void GraphicsBuffer::bind() const
    {
#if !GAMECOE_HAS_DSA
        glBindBuffer(m_target, m_id);
#endif
    }

    void GraphicsBuffer::unbind() const
    {
#if !GAMECOE_HAS_DSA
        glBindBuffer(m_target, 0);
#endif
    }

    void GraphicsBuffer::uploadData(const void* data, size_t size)
    {
        // TODO: introduce usage argument instead of hardcode GL_STATIC_DRAW in the future
#if GAMECOE_HAS_DSA
        glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
#else
        glBufferData(m_target, size, data, GL_STATIC_DRAW);
#endif
    }

    std::uint32_t GraphicsBuffer::id() const
    {
        return m_id;
    }

    std::uint32_t GraphicsBuffer::target() const
    {
        return m_target;
    }

    GraphicsBuffer GraphicsBuffer::createVertexBuffer()
    {
        return GraphicsBuffer(GL_ARRAY_BUFFER);
    }

    GraphicsBuffer GraphicsBuffer::createIndexBuffer()
    {
        return GraphicsBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }
} // namespace gamecoe
