#pragma once

#include <cstddef>
#include <cstdint>

namespace gamecoe
{
    class GraphicsBuffer 
    {
        std::uint32_t m_id;
        std::uint32_t m_target;

        GraphicsBuffer(std::uint32_t target);

    public:
        ~GraphicsBuffer();
        GraphicsBuffer(const GraphicsBuffer&) = delete;
        GraphicsBuffer &operator=(const GraphicsBuffer&) = delete;
        GraphicsBuffer(GraphicsBuffer &&other) noexcept;
        GraphicsBuffer &operator=(GraphicsBuffer &&other) noexcept;

        void bind() const;
        void unbind() const;
        
        void uploadData(const void* data, size_t size);

        std::uint32_t id() const;
        std::uint32_t target() const;

        static GraphicsBuffer createVertexBuffer();
        static GraphicsBuffer createIndexBuffer();
    };

    inline GraphicsBuffer VertexBuffer() { return GraphicsBuffer::createVertexBuffer(); }
    inline GraphicsBuffer IndexBuffer() { return GraphicsBuffer::createIndexBuffer(); }
} // namespace gamecoe
