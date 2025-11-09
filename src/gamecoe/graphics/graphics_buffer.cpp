#include <gamecoe/graphics/graphics_buffer.hpp>
#include <glad/gl.h>

namespace gamecoe
{
    GraphicsBuffer::GraphicsBuffer(unsigned int target) : m_id(0), m_target(target)
    {
        glGenBuffers(1, &m_id);
    }

    GraphicsBuffer::~GraphicsBuffer()
    {
        glDeleteBuffers(1, &m_id);
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

        glDeleteBuffers(1, &m_id);
        m_id = other.m_id;
        m_target = other.m_target;
        other.m_id = 0;
        other.m_target = 0;

        return *this;
    }

    void GraphicsBuffer::bind() const
    {
        glBindBuffer(m_target, m_id);
    }

    void GraphicsBuffer::unbind() const
    {
        glBindBuffer(m_target, 0);
    }

    void GraphicsBuffer::uploadData(const void* data, size_t size)
    {
        // TODO: introduce usage argument instead of hardcode GL_STATIC_DRAW in the future
        glBufferData(m_target, size, data, GL_STATIC_DRAW);
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
