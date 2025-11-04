#include <gamecoe/graphics/vertex_array.hpp>
#include <glad/gl.h>

namespace 
{
    constexpr float triangleVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    constexpr float rectangleVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    constexpr unsigned int rectangleIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    constexpr float cubeVertices[] = {
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };
    constexpr unsigned int cubeIndices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        7, 3, 0, 0, 4, 7,
        1, 5, 6, 6, 2, 1,
        4, 0, 1, 1, 5, 4,
        3, 7, 6, 6, 2, 3
    };
}

namespace gamecoe
{
    std::vector<VertexArray*> VertexArray::s_shapeVAs = {};

    VertexArray::VertexArray(const float *vertices, size_t vertexCount, size_t vertexSize,
                             const unsigned int *indices, size_t indexCount) :  m_id(0), 
                                                                                m_vertexBuffer(VertexBuffer()), 
                                                                                m_vertexCount(vertexCount)
    {
        glGenVertexArrays(1, &m_id);

        if (indices)
        {
            m_indexBuffer.emplace(IndexBuffer());
            m_indexCount = indexCount;
        }

        bind();
        m_vertexBuffer.bind();
        m_vertexBuffer.uploadData(vertices, vertexCount * vertexSize * sizeof(float));

        if (m_indexBuffer)
        {
            m_indexBuffer->bind();
            m_indexBuffer->uploadData(indices, indexCount * sizeof(unsigned int));
        }

        setupVertexAttributes();
        m_vertexBuffer.unbind();
        unbind();
    }

    void VertexArray::setupVertexAttributes()
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // TODO: add support for other attributes such as color, texture, etc..
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

    // TODO: Add support for textured/colored primitives (currently position only)

    const VertexArray &VertexArray::triangle()
    {
        static VertexArray *triangleVA = nullptr;
        if (!triangleVA)
        {
            triangleVA = new VertexArray(triangleVertices, 3, 3);
            s_shapeVAs.push_back(triangleVA);
        }
    
        return *triangleVA;
    }

    const VertexArray &VertexArray::rectangle()
    {
        static VertexArray *rectangleVA = nullptr;
        if (!rectangleVA)
        {
            rectangleVA = new VertexArray(rectangleVertices, 4, 3, rectangleIndices, 6);
            s_shapeVAs.push_back(rectangleVA);
        }
        
        return *rectangleVA;
    }

    const VertexArray &VertexArray::cube()
    {
        static VertexArray *cubeVA = nullptr;
        if (!cubeVA)
        {
            cubeVA = new VertexArray(cubeVertices, 8, 3, cubeIndices, 36);
            s_shapeVAs.push_back(cubeVA);
        }
        
        return *cubeVA;
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
