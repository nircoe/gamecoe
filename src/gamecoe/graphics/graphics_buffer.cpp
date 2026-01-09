#include <gamecoe/graphics/graphics_buffer.hpp>
#include <gamecoe_config.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <cassert>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

namespace gamecoe
{
    GraphicsBuffer::GraphicsBuffer(std::uint32_t target, std::uint32_t bindingPoint) : m_id(0), m_target(target), 
                                                                                       m_usage(0), m_allocated(false)
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

        if (m_target != GL_UNIFORM_BUFFER)
            return;
        
        glBindBufferBase(m_target, bindingPoint, m_id);
        detail::checkAndThrowError("GraphicsBuffer::GraphicsBuffer(): Uniform Buffer:");
        unbind();
    }

    GraphicsBuffer::~GraphicsBuffer()
    {
#if GAMECOE_USE_OPENGL
        glDeleteBuffers(1, &m_id);
#endif
        m_id = 0;
        m_target = 0;
        m_usage = 0;
        m_allocated = false;
    }

    GraphicsBuffer::GraphicsBuffer(GraphicsBuffer &&other) noexcept : m_id(other.m_id), m_target(other.m_target),
                                                                      m_usage(other.m_usage), m_allocated(other.m_allocated)
    {
        other.m_id = 0;
        other.m_target = 0;
        other.m_usage = 0;
        other.m_allocated = false;
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
        m_allocated = other.m_allocated;
        other.m_id = 0;
        other.m_target = 0;
        other.m_usage = 0;
        other.m_allocated = false;

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

    void GraphicsBuffer::uploadData(const void* data, size_t size)
    {
        if (!m_allocated)
        {
#if GAMECOE_HAS_DSA
            glNamedBufferData(m_id, size, nullptr, m_usage);
#else
            bind();
            glBufferData(m_target, size, nullptr, m_usage);
            unbind();
#endif
            detail::checkAndThrowError("GraphicsBuffer::uploadData():");
            m_allocated = true;
        }

#if GAMECOE_HAS_DSA
        glNamedBufferSubData(m_id, 0, size, data);
        detail::checkAndThrowError("GraphicsBuffer::uploadData():");
#else
        bind();
        glBufferSubData(m_target, 0, size, data);
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

    GraphicsBuffer GraphicsBuffer::createUniformBuffer(std::uint32_t bindingPoint)
    {
        return GraphicsBuffer(GL_UNIFORM_BUFFER, bindingPoint);
    }
} // namespace gamecoe
