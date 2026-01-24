#pragma once
#include <gamecoe/graphics/graphics_buffer.hpp>
#include <optional>
#include <vector>
#include <cstdint>

namespace gamecoe
{
    class VertexArray
    {
        static std::vector<VertexArray*> s_shapeVAs;

        std::uint32_t m_id;
        GraphicsBuffer m_vertexBuffer;
        std::optional<GraphicsBuffer> m_indexBuffer;
        size_t m_vertexCount;
        size_t m_indexCount;
        // should we add vertexCount, vertexSize, indexCount members?

        VertexArray() = delete;
        VertexArray(const float *vertices, size_t vertexCount, size_t vertexSize,
                    const std::uint32_t *indices = nullptr, size_t indexCount = 0);
        
        void setupVertexAttributes();

    public:
        
        ~VertexArray();
        VertexArray(const VertexArray&) = delete;
        VertexArray &operator=(const VertexArray&) = delete;
        VertexArray(VertexArray &&other) noexcept;
        VertexArray &operator=(VertexArray &&other) noexcept;

        void bind() const;
        void unbind() const;

        size_t vertexCount() const;
        size_t indexCount() const;
        bool hasIndices() const;

        static const VertexArray &triangle();
        static const VertexArray &rectangle();
        static const VertexArray &box();
        static void destroyShapeVAs();
    };
} // namespace gamecoe