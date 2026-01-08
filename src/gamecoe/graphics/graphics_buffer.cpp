#include <gamecoe/graphics/graphics_buffer.hpp>
#include <gamecoe_config.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <cassert>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

namespace gamecoe
{
    GraphicsBuffer::GraphicsBuffer(std::uint32_t target) : m_id(0), m_target(target), m_usage(0)
    {
#if GAMECOE_HAS_DSA
        glCreateBuffers(1, &m_id);
#elif GAMECOE_USE_OPENGL
#if !GAMECOE_HAS_UBO
        if (m_target == GL_UNIFORM_BUFFER)
            detail::throwError("GraphicsBuffer::GraphicsBuffer(): OpenGL version doesn't support Uniform Buffer");
#endif
        glGenBuffers(1, &m_id);
#else
        detail::throwError("GraphicsBuffer::GraphicsBuffer(): Only OpenGL supported at the moment");
#endif
        if (m_id == 0)
            detail::throwError("GraphicsBuffer::GraphicsBuffer(): Could not generate buffer");

        m_usage = m_target == GL_UNIFORM_BUFFER ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    }

    GraphicsBuffer::~GraphicsBuffer()
    {
#if GAMECOE_USE_OPENGL
        glDeleteBuffers(1, &m_id);
#endif
        m_id = 0;
        m_target = 0;
        m_usage = 0;
    }

    GraphicsBuffer::GraphicsBuffer(GraphicsBuffer &&other) noexcept : m_id(other.m_id), m_target(other.m_target), m_usage(other.m_usage)
    {
        other.m_id = 0;
        other.m_target = 0;
        other.m_usage = 0;
    }

    GraphicsBuffer &GraphicsBuffer::operator=(GraphicsBuffer &&other) noexcept
    {
        if (this == &other) return *this;

#if GAMECOE_USE_OPENGL
        glDeleteBuffers(1, &m_id);
#endif
        m_id = other.m_id;
        m_target = other.m_target;
        m_usage = other.m_usage;
        other.m_id = 0;
        other.m_target = 0;
        other.m_usage = 0;

        return *this;
    }

    void GraphicsBuffer::bind() const
    {
#if !GAMECOE_HAS_DSA
        glBindBuffer(m_target, m_id);
        detail::checkAndThrowError("GraphicsBuffer::bind():");
#endif
    }

    void GraphicsBuffer::unbind() const
    {
#if !GAMECOE_HAS_DSA
        glBindBuffer(m_target, 0);
        detail::checkAndThrowError("GraphicsBuffer::unbind():");
#endif
    }

    void GraphicsBuffer::bindBase(std::uint32_t bindingPoint) const
    {
        if (m_target != GL_UNIFORM_BUFFER)
            detail::throwError("GraphicsBuffer::bindBase(): Use this method only for Uniform Buffers");
        
        bind();
        glBindBufferBase(m_target, bindingPoint, m_id);
        detail::checkAndThrowError("GraphicsBuffer::bindBase():");
        unbind();
    }

    void GraphicsBuffer::uploadData(const void* data, size_t size)
    {
#if GAMECOE_HAS_DSA
        glNamedBufferData(m_id, size, data, m_usage);
        detail::checkAndThrowError("GraphicsBuffer::uploadData():");
#else
        bind();
        if (m_target == GL_UNIFORM_BUFFER) bindBase(); // when should I call bindBase?
        glBufferData(m_target, size, data, m_usage);
        detail::checkAndThrowError("GraphicsBuffer::uploadData():");
        unbind();
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

    GraphicsBuffer GraphicsBuffer::createUniformBuffer()
    {
        return GraphicsBuffer(GL_UNIFORM_BUFFER);
    }
} // namespace gamecoe
