#include <gamecoe/graphics/vertex_array.hpp>
#include <gamecoe/utils/geometry.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_OPENGL
#include <glad/gl.h>
#endif

namespace gamecoe
{
    std::vector<VertexArray*> VertexArray::s_shapeVAs = {};

    VertexArray::VertexArray(const float *vertices, size_t vertexCount, size_t vertexSize,
                             const std::uint32_t *indices, size_t indexCount) :  
        m_id(0), 
        m_vertexBuffer(VertexBuffer()), 
        m_vertexCount(vertexCount)
    {
#if GAMECOE_HAS_DSA
        glCreateVertexArrays(1, &m_id);
#else
        glGenVertexArrays(1, &m_id);
#endif

        if (indices)
        {
            m_indexBuffer.emplace(IndexBuffer());
            m_indexCount = indexCount;
        }

#if !GAMECOE_HAS_DSA
        bind();
#endif
        m_vertexBuffer.uploadData(vertices, vertexCount * vertexSize * sizeof(float));

        if (m_indexBuffer)
            m_indexBuffer->uploadData(indices, indexCount * sizeof(std::uint32_t));

        setupVertexAttributes();
#if !GAMECOE_HAS_DSA
        unbind();
#endif
    }

    void VertexArray::setupVertexAttributes()
    {
#if GAMECOE_HAS_DSA
        glVertexArrayVertexBuffer(m_id, 0, m_vertexBuffer.id(), 0, 3 * sizeof(float));
        glVertexArrayAttribFormat(m_id, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_id, 0, 0);
        glEnableVertexArrayAttrib(m_id, 0);

        if (m_indexBuffer)
            glVertexArrayElementBuffer(m_id, m_indexBuffer->id());
#else
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
#endif
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
        m_vertexCount = 0;
        m_indexCount = 0;
    }

    VertexArray::VertexArray(VertexArray &&other) noexcept :    m_id(other.m_id), 
                                                                m_vertexBuffer(std::move(other.m_vertexBuffer)),
                                                                m_indexBuffer(std::move(other.m_indexBuffer)), 
                                                                m_vertexCount(other.m_vertexCount), 
                                                                m_indexCount(other.m_indexCount)
    {
        other.m_id = 0;
        other.m_vertexCount = 0;
        other.m_indexCount = 0;
    }

    VertexArray &VertexArray::operator=(VertexArray &&other) noexcept
    {
        if(this == &other) return *this;

        glDeleteVertexArrays(1, &m_id);
        m_id = other.m_id;
        m_vertexBuffer = std::move(other.m_vertexBuffer);
        m_indexBuffer = std::move(other.m_indexBuffer);
        m_vertexCount = other.m_vertexCount;
        m_indexCount = other.m_indexCount;

        other.m_id = 0;
        other.m_vertexCount = 0;
        other.m_indexCount = 0;

        return *this;
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(m_id);
    }

    void VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    size_t VertexArray::vertexCount() const
    {
        return m_vertexCount;
    }

    size_t VertexArray::indexCount() const
    {
        return m_indexCount;
    }

    bool VertexArray::hasIndices() const
    {
        return m_indexBuffer.has_value();
    }

    const VertexArray &VertexArray::triangle()
    {
        static VertexArray *triangleVA = nullptr;
        if (!triangleVA)
        {
            static constexpr auto vertices = geometry::triangle::verticesFlat();
            triangleVA = new VertexArray(vertices.data(), vertices.size() / 3, 3);
            s_shapeVAs.push_back(triangleVA);
        }
    
        return *triangleVA;
    }

    const VertexArray &VertexArray::rectangle()
    {
        static VertexArray *rectangleVA = nullptr;
        if (!rectangleVA)
        {
            static constexpr auto vertices = geometry::rectangle::verticesFlat();
            static constexpr auto indices = geometry::rectangle::indices();
            rectangleVA = new VertexArray(vertices.data(), vertices.size() / 3, 3, indices.data(), indices.size());
            s_shapeVAs.push_back(rectangleVA);
        }
        
        return *rectangleVA;
    }

    const VertexArray &VertexArray::box()
    {
        static VertexArray *boxVA = nullptr;
        if (!boxVA)
        {
            static constexpr auto vertices = geometry::box::verticesFlat();
            static constexpr auto indices = geometry::box::indices();
            boxVA = new VertexArray(vertices.data(), vertices.size() / 3, 3, indices.data(), indices.size());
            s_shapeVAs.push_back(boxVA);
        }
        
        return *boxVA;
    }

    void VertexArray::destroyShapeVAs()
    {
        for(const VertexArray *va : s_shapeVAs)
        {
            if (va)
                delete va;
        }
        s_shapeVAs.clear();
    }
} // namespace gamecoe
